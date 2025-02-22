import time
import argparse

from droneSwarmInterfaceServer import DroneSwarmInterfaceServer, Protocol
import cflib.crtp
from cflib.crazyflie.swarm import CachedCfFactory
from cflib.crazyflie.swarm import Swarm
from cflib.crazyflie.syncCrazyflie import SyncCrazyflie
from cflib.crazyflie.high_level_commander import HighLevelCommander
from cflib.crazyflie.syncLogger import SyncLogger
from cflib.crazyflie.log import LogConfig
from simple_pid import PID
import datetime
import threading

stop_time = 3.0
default_movement_speed_s = 1.0
landing_height = 0.2
landing_time = 3.0
takeoff_height = 0.8
takeoff_time = 3.0

drone_positions: dict[str, Protocol.Position] = dict()
drone_targets: dict[str, Protocol.Position] = dict()
drone_states: dict[str, Protocol.DroneState] = dict()
drone_operations: dict[str, Protocol.DroneOperation] = dict()
uri_index_mapping: dict[str, int] = dict()
index_uri_mapping: dict[int, str] = dict()

def calculate_movement_time_setpoint(current_position: Protocol.Position, target_position: Protocol.Position):
    return max(current_position.euclidean_distance(target_position) / 0.4 * 1.0, 0.2)
def update_drone(scf: SyncCrazyflie):
    global drone_positions
    global drone_targets
    global drone_states
    global drone_operations
    global uri_index_mapping
    global index_uri_mapping
    global takeoff_height
    
    commander: HighLevelCommander = scf.cf.high_level_commander
    command_start = datetime.datetime.now()
    operation_time = 0.0

    log_config = LogConfig(name='stateEstimate', period_in_ms=100)
    log_config.add_variable('stateEstimate.x', 'float')
    log_config.add_variable('stateEstimate.y', 'float')
    log_config.add_variable('stateEstimate.z', 'float')
    log_config.add_variable('pm.batteryLevel', 'uint8_t')
    
    last_battery_level = 0.0

    with SyncLogger(scf, log_config) as logger:
        lastOperation = drone_operations[scf.cf.link_uri]
        for entry in logger:
            x = entry[1]['stateEstimate.x']
            y = entry[1]['stateEstimate.y']
            z = entry[1]['stateEstimate.z']
            battery_level = entry[1]['pm.batteryLevel']
            if battery_level != last_battery_level:
                print('Battery level of drone with URI ' + scf.cf.link_uri + ': ' + str(battery_level))
                last_battery_level = battery_level
            drone_positions[scf.cf.link_uri] = Protocol.Position(x, y, z, 0.0)

            difftime = (datetime.datetime.now() - command_start)
            difftime_ms = difftime.seconds * 1000 + difftime.microseconds / 1000.0

            if drone_states[scf.cf.link_uri] == Protocol.DroneState.DRONE_IDLE:
                #Handle Idle state
                if drone_operations[scf.cf.link_uri] == Protocol.DroneOperation.DRONE_OPERATION_TAKE_OFF:
                    #If we want to take of switch the state to "taking off" and start
                    print(scf.cf.link_uri + ' takeoff')
                    drone_states[scf.cf.link_uri] = Protocol.DroneState.DRONE_TAKING_OFF
                    command_start = datetime.datetime.now()
                    print('High level commander: Take off in ' + str(takeoff_time) + "s to height: "+ str(takeoff_height))
                    commander.takeoff(takeoff_height, takeoff_time)
            elif drone_states[scf.cf.link_uri] == Protocol.DroneState.DRONE_TAKING_OFF:
                #Handle taking off state
                if drone_operations[scf.cf.link_uri] == Protocol.DroneOperation.DRONE_OPERATION_FAST_STOP:
                    #On fast stop, stop immediately
                    print(scf.cf.link_uri + ' fast stop')
                    drone_states[scf.cf.link_uri] = Protocol.DroneState.DRONE_STOPPING
                    command_start = datetime.datetime.now()
                    scf.cf.high_level_commander.stop()
                elif difftime_ms > takeoff_time * 1000.0 + takeoff_time * 1000.0 / 4.0:
                    if drone_positions[scf.cf.link_uri].z < takeoff_height - 0.3:
                        print(scf.cf.link_uri + ' fast stop: height not reached')
                        drone_states[scf.cf.link_uri] = Protocol.DroneState.DRONE_STOPPING
                        command_start = datetime.datetime.now()
                        scf.cf.high_level_commander.stop()
                    else:
                        print(scf.cf.link_uri + ' hovering')
                        drone_states[scf.cf.link_uri] = Protocol.DroneState.DRONE_HOVERING
            elif drone_states[scf.cf.link_uri] == Protocol.DroneState.DRONE_HOVERING:
                #Handle hover state
                if drone_operations[scf.cf.link_uri] == Protocol.DroneOperation.DRONE_OPERATION_MOVE:
                    #If we want to move initialize move operation
                    print(scf.cf.link_uri + ' move')
                    command_start = datetime.datetime.now()
                    time_setpoint = calculate_movement_time_setpoint(drone_positions[scf.cf.link_uri], drone_targets[scf.cf.link_uri])
                    commander.go_to(drone_targets[scf.cf.link_uri].x, drone_targets[scf.cf.link_uri].y, drone_targets[scf.cf.link_uri].z, drone_targets[scf.cf.link_uri].yaw, time_setpoint)
                    operation_time = time_setpoint * 1.5
                    drone_states[scf.cf.link_uri] = Protocol.DroneState.DRONE_MOVING
                elif drone_operations[scf.cf.link_uri] == Protocol.DroneOperation.DRONE_OPERATION_LAND:
                    #Initialize landing operation
                    print(scf.cf.link_uri + ' land')
                    drone_states[scf.cf.link_uri] = Protocol.DroneState.DRONE_LANDING
                    command_start = datetime.datetime.now()
                    commander.land(landing_height, landing_time)
                elif drone_operations[scf.cf.link_uri] == Protocol.DroneOperation.DRONE_OPERATION_FAST_STOP:
                    #Fast stop -> stop immediately
                    print(scf.cf.link_uri + ' fast stop')
                    drone_states[scf.cf.link_uri] = Protocol.DroneState.DRONE_STOPPING
                    command_start = datetime.datetime.now()
                    commander.stop()
            elif drone_states[scf.cf.link_uri] == Protocol.DroneState.DRONE_MOVING:
                fastStop = False
                for u, position in drone_positions.items():
                    if u != uri and position.euclidean_distance(drone_positions[uri]) <= 0.2:
                        print("Fast stop due to getting too close! Additional information:")
                        print('Positions: ', end='')
                        print(drone_positions[u], end=' ')
                        print(drone_positions[uri])
                        print('Targets: ', end='')
                        print(drone_targets[u], end=' ')
                        print(drone_targets[uri])
                        fastStop = True
                if drone_operations[scf.cf.link_uri] == Protocol.DroneOperation.DRONE_OPERATION_FAST_STOP or fastStop:
                    #On fast stop, stop immediately
                    print(scf.cf.link_uri + ' fast stop')
                    drone_states[scf.cf.link_uri] = Protocol.DroneState.DRONE_STOPPING
                    command_start = datetime.datetime.now()
                    commander.stop()
                elif drone_operations[scf.cf.link_uri] == Protocol.DroneOperation.DRONE_OPERATION_LAND:
                    print(scf.cf.link_uri + ' land')
                    drone_states[scf.cf.link_uri] = Protocol.DroneState.DRONE_LANDING
                    command_start = datetime.datetime.now()
                    commander.land(landing_height, landing_time)
                elif difftime_ms > operation_time * 1000:
                    #Assume we are hovering after the set time
                    print(scf.cf.link_uri + ' hovering')
                    drone_states[scf.cf.link_uri] = Protocol.DroneState.DRONE_HOVERING
            elif drone_states[scf.cf.link_uri] == Protocol.DroneState.DRONE_LANDING:
                if drone_operations[scf.cf.link_uri] == Protocol.DroneOperation.DRONE_OPERATION_FAST_STOP:
                    #On fast stop, stop immediately
                    print(scf.cf.link_uri + ' fast stop')
                    drone_states[scf.cf.link_uri] = Protocol.DroneState.DRONE_STOPPING
                    command_start = datetime.datetime.now()
                    commander.stop()
                elif difftime_ms > landing_time * 1000 + landing_time * 1000.0 / 4.0:
                    #Assume we are hovering after the set time
                    print(scf.cf.link_uri + ' stopping')
                    drone_states[scf.cf.link_uri] = Protocol.DroneState.DRONE_STOPPING
                    command_start = datetime.datetime.now()
                    commander.stop()
            elif drone_states[scf.cf.link_uri] == Protocol.DroneState.DRONE_STOPPING:
                if difftime_ms > stop_time * 1000:
                    print(scf.cf.link_uri + ' idle')
                    drone_states[scf.cf.link_uri] = Protocol.DroneState.DRONE_IDLE
            else:
                #Impossible case, but anyway...
                print(scf.cf.link_uri + ' Error state: Stop')
                commander.stop()
                time.sleep(3.0)
                raise(Exception('Error: Invalid state of drone with URI ' + scf.cf.link_uri))
            
            if lastOperation == drone_operations[scf.cf.link_uri] and drone_operations[scf.cf.link_uri] != Protocol.DroneOperation.DRONE_OPERATION_NONE:
                #Operation hasn't changed -> Set to None
                print('Reset operation')
                drone_operations[scf.cf.link_uri] = Protocol.DroneOperation.DRONE_OPERATION_NONE
            lastOperation = drone_operations[scf.cf.link_uri]
            time.sleep(0.1)

last_active_targets = dict()

def update_virtual_drones(uri: str):
    global drone_positions
    global drone_targets
    global drone_states
    global drone_operations
    global uri_index_mapping
    global index_uri_mapping
    global takeoff_height
    global last_active_targets

    pidX = PID(0.1, 0.0, 0.0, setpoint=1)
    pidY = PID(0.1, 0.0, 0.0, setpoint=1)
    pidZ = PID(0.1, 0.0, 0.0, setpoint=1)

    command_start = datetime.datetime.now()
    operation_time = 0.0
    
    lastOperation = drone_operations[uri]
    while True:          
        difftime = (datetime.datetime.now() - command_start)
        difftime_ms = difftime.seconds * 1000 + difftime.microseconds / 1000.0

        if drone_states[uri] == Protocol.DroneState.DRONE_IDLE:
            #Handle Idle state
            if drone_operations[uri] == Protocol.DroneOperation.DRONE_OPERATION_TAKE_OFF:
                #If we want to take of switch the state to "taking off" and start
                print(uri + ' takeoff')
                drone_states[uri] = Protocol.DroneState.DRONE_TAKING_OFF
                command_start = datetime.datetime.now()
                print('High level commander: Take off in ' + str(takeoff_time) + "s to height: "+ str(takeoff_height))
        elif drone_states[uri] == Protocol.DroneState.DRONE_TAKING_OFF:
            #Handle taking off state
            if drone_operations[uri] == Protocol.DroneOperation.DRONE_OPERATION_FAST_STOP:
                #On fast stop, stop immediately
                print(uri + ' fast stop')
                drone_states[uri] = Protocol.DroneState.DRONE_STOPPING
                command_start = datetime.datetime.now()
            elif difftime_ms > takeoff_time * 1000.0 + takeoff_time * 1000.0 / 4.0:
                if drone_positions[uri].z < takeoff_height - 0.3:
                    print(uri + ' fast stop: height not reached')
                    drone_states[uri] = Protocol.DroneState.DRONE_STOPPING
                    command_start = datetime.datetime.now()
                else:
                    print(uri + ' hovering')
                    drone_states[uri] = Protocol.DroneState.DRONE_HOVERING
            else:
                # Factor 10 due to update interval of 100ms
                drone_positions[uri] = Protocol.Position(drone_positions[uri].x, drone_positions[uri].y, drone_positions[uri].z + (takeoff_height / takeoff_time) / 10.0, 0.0)
                print(uri + ': Updated position for takeoff to ' + str(drone_positions[uri]))
        elif drone_states[uri] == Protocol.DroneState.DRONE_HOVERING:
            #Handle hover state
            if drone_operations[uri] == Protocol.DroneOperation.DRONE_OPERATION_MOVE:
                #If we want to move initialize move operation
                print(uri + ' move from ' + str(drone_positions[uri]) + ' to ' + str(drone_targets[uri]))
                command_start = datetime.datetime.now()
                pidX = PID(0.2, 0.1, 0.0, setpoint=drone_targets[uri].x)
                pidY = PID(0.2, 0.1, 0.0, setpoint=drone_targets[uri].y)
                pidZ = PID(0.2, 0.1, 0.0, setpoint=drone_targets[uri].z)

                time_setpoint = calculate_movement_time_setpoint(drone_positions[uri], drone_targets[uri])
                last_active_targets[uri] = (Protocol.Position(drone_targets[uri].x, drone_targets[uri].y, drone_targets[uri].z, 0.0), time_setpoint)
                print('Target vector: ', end='')
                print(last_active_targets)
                operation_time = time_setpoint * 1.5
                drone_states[uri] = Protocol.DroneState.DRONE_MOVING
            elif drone_operations[uri] == Protocol.DroneOperation.DRONE_OPERATION_LAND:
                #Initialize landing operation
                print(uri + ' land')
                drone_states[uri] = Protocol.DroneState.DRONE_LANDING
                command_start = datetime.datetime.now()
            elif drone_operations[uri] == Protocol.DroneOperation.DRONE_OPERATION_FAST_STOP:
                #Fast stop -> stop immediately
                print(uri + ' fast stop')
                drone_states[uri] = Protocol.DroneState.DRONE_STOPPING
                command_start = datetime.datetime.now()
        elif drone_states[uri] == Protocol.DroneState.DRONE_MOVING:
            fastStop = False
            for u, position in drone_positions.items():
                if u != uri and position.euclidean_distance(drone_positions[uri]) <= 0.00:
                    print("Fast stop due to getting too close! Additional information:")
                    print('Positions: ', end='')
                    print(drone_positions[u], end=' ')
                    print(drone_positions[uri])
                    print('Targets: ', end='')
                    print(drone_targets[u], end=' ')
                    print(drone_targets[uri])
                    fastStop = True
            if drone_operations[uri] == Protocol.DroneOperation.DRONE_OPERATION_FAST_STOP or fastStop:
                #On fast stop, stop immediately
                print(uri + ' fast stop')
                drone_states[uri] = Protocol.DroneState.DRONE_STOPPING
                command_start = datetime.datetime.now()
            elif drone_operations[uri] == Protocol.DroneOperation.DRONE_OPERATION_LAND:
                print(uri + ' land')
                drone_states[uri] = Protocol.DroneState.DRONE_LANDING
                command_start = datetime.datetime.now()
            elif difftime_ms > operation_time * 1000:
                #Assume we are hovering after the set time
                print(uri + ' hovering')
                drone_states[uri] = Protocol.DroneState.DRONE_HOVERING
            else:
                #Factor 10 due to update interval of 100ms
                cX = pidX(drone_positions[uri].x)
                cY = pidY(drone_positions[uri].y)
                cZ = pidZ(drone_positions[uri].z)

                drone_positions[uri] = Protocol.Position(drone_positions[uri].x + cX,
                                                            drone_positions[uri].y + cY,
                                                            drone_positions[uri].z + cZ, 0.0)

                print(uri + ': Drone position for movement updated to ' + str(drone_positions[uri]))
        elif drone_states[uri] == Protocol.DroneState.DRONE_LANDING:
            if drone_operations[uri] == Protocol.DroneOperation.DRONE_OPERATION_FAST_STOP:
                #On fast stop, stop immediately
                print(uri + ' fast stop')
                drone_states[uri] = Protocol.DroneState.DRONE_STOPPING
                command_start = datetime.datetime.now()
            elif difftime_ms > landing_time * 1000 + landing_time * 1000.0 / 4.0:
                #Assume we are hovering after the set time
                print(uri + ' stopping')
                drone_states[uri] = Protocol.DroneState.DRONE_STOPPING
                command_start = datetime.datetime.now()
            else:
                #Factor 10 due to update interval of 100ms
                drone_positions[uri] = Protocol.Position(drone_positions[uri].x, drone_positions[uri].y, drone_positions[uri].z - (drone_positions[uri].z / 10 / (max(landing_time - difftime_ms, 0.001))), 0.0)
        elif drone_states[uri] == Protocol.DroneState.DRONE_STOPPING:
            if difftime_ms > stop_time * 1000:
                print(uri + ' idle')
                drone_states[uri] = Protocol.DroneState.DRONE_IDLE
            else:
                drone_positions[uri] = Protocol.Position(0.0, 0.0, 0.0, 0.0)
                print(uri + ": Drone position for stopping updated to " + str(drone_positions[uri]))
        else:
            #Impossible case, but anyway...
            print(uri + ' Error state: Stop')
            time.sleep(3.0)
            raise(Exception('Error: Invalid state of drone with URI ' + uri))
            
        if lastOperation == drone_operations[uri] and drone_operations[uri] != Protocol.DroneOperation.DRONE_OPERATION_NONE:
            #Operation hasn't changed -> Set to None
            print('Reset operation')
            drone_operations[uri] = Protocol.DroneOperation.DRONE_OPERATION_NONE
        lastOperation = drone_operations[uri]
        time.sleep(0.1)

def update_server_info(server : DroneSwarmInterfaceServer):
    global drone_positions
    global drone_targets
    global drone_states
    global drone_operations
    global uri_index_mapping
    global index_uri_mapping
    
    last_server_request = server.drone_operations
    while True:
        #Update states and positions with the CrazyFlie-API information
        server.drone_states = {uri_index_mapping[k] : v for k,v in drone_states.items()}
        server.drone_positions = {uri_index_mapping[k] : v for k,v in drone_positions.items()}
        
        #Update targets with values from server
        drone_targets = {index_uri_mapping[k] : v for k,v in server.drone_targets.items()}
        
        if last_server_request != server.drone_operations:
            drone_operations = {index_uri_mapping[k] : v for k,v in server.drone_operations.items()}
        else:
            server.drone_operations = drone_operations
        
        states = {v for k,v in server.drone_states.items()}
        
        if any(s == Protocol.DroneState.DRONE_MOVING for s in states):
            server.swarm_state = Protocol.SwarmState.SWARM_MOVING
        elif any(s == Protocol.DroneState.DRONE_TAKING_OFF for s in states):
            server.swarm_state = Protocol.SwarmState.SWARM_TAKING_OFF
        elif any(s == Protocol.DroneState.DRONE_LANDING for s in states):
            server.swarm_state = Protocol.SwarmState.SWARM_LANDING
        elif any(s == Protocol.DroneState.DRONE_STOPPING for s in states):
            server.swarm_state = Protocol.SwarmState.SWARM_STOPPING
        elif all(s == Protocol.DroneState.DRONE_HOVERING for s in states):
            server.swarm_state = Protocol.SwarmState.SWARM_HOVERING
        elif all(s == Protocol.DroneState.DRONE_IDLE for s in states):
            server.swarm_state = Protocol.SwarmState.SWARM_IDLE
        else:
            print('Error! Swarm has invalid state!')
            server.swarm_state = Protocol.SwarmState.SWARM_STOPPING
        
        last_server_request = server.drone_operations
        time.sleep(0.015)

def position(s):
    try:
        x, y, z = map(int, s.split(','))
        return x, y, z
    except:
        raise argparse.ArgumentTypeError("Missing coordinate")

if __name__ == "__main__":
    CLI=argparse.ArgumentParser()
    CLI.add_argument("--drones", help='Real drones URI (only end number after radio://0/80/2M/E7E7E7E7E)', nargs="*", type=int, default=[],)
    CLI.add_argument("--virtual", help='Virtual drones identified by their (x,y,z) coordinates', nargs="*", type=position, default=[],)
    args = CLI.parse_args()

    #swarm_uris = ['radio://' + ('0/80' if i % 2 == 1 else '1/100') + '/2M/E7E7E7E7E' + hex(i)[-1] for i in args.drones]
    swarm_uris = ['radio://0/80/2M/E7E7E7E7E' + hex(i)[-1] for i in args.drones]
    virtual_drones = dict()
    cnt = 0
    for v in args.virtual:
        virtual_drones[str(cnt)] = v
        cnt = cnt+1

    if len(swarm_uris) + len(virtual_drones) < 2:
        print('Not enough CrazyFlies! At minimum 2 are neccessary to form a "swarm"!')
        exit(3)

    host, port = '', 12345

    try:
        cflib.crtp.init_drivers()
    except:
        print('Could not intialize the CrazyFlie driver!')
        exit(2)
    with Swarm(swarm_uris, factory=CachedCfFactory(rw_cache='./cache')) as swarm:
        try:
            swarm.reset_estimators()
        except:
            print('Could not reset position estimators! (Are the CrazyFlies even connected?)')
            exit(4)
        print('Initial positions: ', end='')

        drone_positions = dict()

        for uri in swarm_uris:
            startTime = time.time()
            positions = []
            print(uri + ': ')
            while(time.time() - startTime < 2.5):
                p = swarm.get_estimated_positions()
                p = p[uri]
                positions = positions + [(p.x, p.y, p.z)]
                print((p.x, p.y, p.z), end=' ')
                time.sleep(0.5)
            print()
            z = list(zip(*positions))

            if max(z[0]) - min(z[0]) > 0.1:
                print("Estimation (X) too bad!")
                exit(1)
            elif max(z[1]) - min(z[1]) > 0.1:
                print("Estimation (Y) too bad!")
                exit(1)
            elif max(z[2]) - min(z[2]) > 0.1:
                print("Estimation (Z) too bad!")
                exit(1)
            else:
                uri_index_mapping[uri] = swarm_uris.index(uri)
                index_uri_mapping[uri_index_mapping[uri]] = uri
                drone_positions[uri] = Protocol.Position(positions[-1][0], positions[-1][1], positions[-1][2], 0.0)
        for uri, position_tuple in virtual_drones.items():
            uri_index_mapping[uri] = list(virtual_drones).index(uri) + len(swarm_uris)
            index_uri_mapping[uri_index_mapping[uri]] = uri
            drone_positions[uri] = Protocol.Position(position_tuple[0], position_tuple[1], position_tuple[2], 0.0)

        server = DroneSwarmInterfaceServer(port, host, {uri_index_mapping[d]: drone_positions[d] for d, k in drone_positions.items()})
        drone_operations = {index_uri_mapping[k] : v for k, v in server.drone_operations.items()}
        drone_positions = {index_uri_mapping[k] : v for k, v in server.drone_positions.items()}
        drone_states = {index_uri_mapping[k] : v for k, v in server.drone_states.items()}
        drone_targets = {index_uri_mapping[k] : v for k, v in server.drone_targets.items()}
        
        update_thread = threading.Thread(target=update_server_info, args=(server,))
        update_thread.start()
        
        for uri, position_tuple in virtual_drones.items():
            virtual_drone_thread = threading.Thread(target=update_virtual_drones, args=(uri,))
            virtual_drone_thread.start()

        swarm.parallel_safe(update_drone)