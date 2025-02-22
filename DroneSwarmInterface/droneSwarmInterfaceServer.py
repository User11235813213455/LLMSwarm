import socket
import threading
from enum import IntEnum
import math
import time

class Protocol:
    class Message(IntEnum):
        REGISTER_SWARM_NOTIFICATION_REQUEST=1
        REGISTER_SWARM_NOTIFICATION_RESPONSE=2
        SWARM_STATE_NOTIFICATION_MESSAGE=3
        SET_DRONE_TARGET_POSITION_REQUEST=4
        SET_DRONE_TARGET_POSITION_RESPONSE=5
        SWARM_OPERATIONS_REQUEST=6
        SWARM_OPERATIONS_RESPONSE=7
        DRONE_OPERATIONS_REQUEST=8
        DRONE_OPERATIONS_RESPONSE=9

    class DroneOperation(IntEnum):
        DRONE_OPERATION_NONE=0
        DRONE_OPERATION_TAKE_OFF=1
        DRONE_OPERATION_LAND=2
        DRONE_OPERATION_FAST_STOP=3
        DRONE_OPERATION_MOVE=4

    class SwarmOperation(IntEnum):
        SWARM_OPERATION_TAKEOFF=0
        SWARM_OPERATION_LAND=1
        SWARM_OPERATION_MOVE=2
        SWARM_OPERATION_FAST_STOP=3

    class DroneState(IntEnum):
        DRONE_IDLE=0
        DRONE_TAKING_OFF=1
        DRONE_HOVERING=2
        DRONE_MOVING=3
        DRONE_LANDING=4
        DRONE_STOPPING=5

    class SwarmState(IntEnum):
        SWARM_IDLE=0
        SWARM_TAKING_OFF=1
        SWARM_HOVERING=2
        SWARM_MOVING=3
        SWARM_LANDING=4
        SWARM_STOPPING=5

    class Position:
        def __init__(self, x, y, z, yaw):
            self.x = x
            self.y = y
            self.z = z
            self.yaw = yaw
        def __str__(self,):
            return str({'x': self.x, 'y': self.y, 'z': self.z, 'yaw': self.yaw})
        def __repr__(self,):
            return self.__str__()
        def euclidean_distance(self, other):
            return math.sqrt((self.x - other.x)**2 + (self.y - other.y)**2 + (self.z - other.z)**2)

    @staticmethod
    def append_uint8(data: list[int], value: int) -> None:
        if value > 0xFF:
            raise(Exception('Tried to parse data as uint8 which does not fit into a uint8!'))
        data.append((value & 0xFF))

    @staticmethod
    def append_uint16(data: list[int], value: int) -> None:
        if value > 0xFFFF:
            raise(Exception('Tried to parse data as uint16 which does not fit into a uint16!'))
        
        data.append((value & 0x00FF))
        data.append((value & 0xFF00) >> 8)
    
    @staticmethod
    def append_uint32(data: list[int], value: int) -> None:
        if value > 0xFFFFFFFF:
            raise(Exception('Tried to parse data as uint32 which does not fit into a uint32!'))
        
        data.append((value & 0x000000FF))
        data.append((value & 0x0000FF00) >> 8)
        data.append((value & 0x00FF0000) >> 16)
        data.append((value & 0xFF000000) >> 24)

    @staticmethod
    def extract_uint8(data: bytes) -> int:
        return int(data[0]) if data[0] <= 127 else data[0] - 256
    
    @staticmethod
    def extract_uint16(data: bytes) -> int:
        unsigned = int(data[1]) << 8 | int(data[0])
        return unsigned if unsigned <= 32768 else unsigned - 65536
    
    @staticmethod
    def extract_uint32(data: bytes) -> int:
        unsigned = int(data[3]) << 24 | int(data[2]) << 16 | int(data[1]) << 8 | int(data[0])
        return unsigned if unsigned <= 2147483648 else unsigned - 4294967296

    @staticmethod
    def create_swarm_notification_message(drone_positions: dict[int, Position], drone_targets: dict[int, Position], drone_states: dict[int, SwarmState], drone_operations: dict[int, DroneOperation], swarm_state: SwarmState) -> bytes:
        raw_data = []
        Protocol.append_uint8(raw_data, int(Protocol.Message.SWARM_STATE_NOTIFICATION_MESSAGE))
        Protocol.append_uint8(raw_data, len(drone_positions))

        for drone, position in drone_positions.items():
            Protocol.append_uint16(raw_data, drone)
            Protocol.append_uint32(raw_data, math.floor(position.x * 10000))
            Protocol.append_uint32(raw_data, math.floor(position.y * 10000))
            Protocol.append_uint32(raw_data, math.floor(position.z * 10000))
            Protocol.append_uint32(raw_data, math.floor(position.yaw * 10000))
        
        Protocol.append_uint8(raw_data, len(drone_targets))

        for drone, position in drone_targets.items():
            Protocol.append_uint16(raw_data, drone)
            Protocol.append_uint32(raw_data, math.floor(position.x * 10000))
            Protocol.append_uint32(raw_data, math.floor(position.y * 10000))
            Protocol.append_uint32(raw_data, math.floor(position.z * 10000))
            Protocol.append_uint32(raw_data, math.floor(position.yaw * 10000))

        Protocol.append_uint8(raw_data, len(drone_states))

        for drone, state in drone_states.items():
            Protocol.append_uint16(raw_data, drone)
            Protocol.append_uint8(raw_data, int(state))

        Protocol.append_uint8(raw_data, len(drone_operations))

        for drone, operation in drone_states.items():
            Protocol.append_uint16(raw_data, drone)
            Protocol.append_uint8(raw_data, int(operation))

        Protocol.append_uint8(raw_data, swarm_state)

        return bytes(raw_data)

    def create_register_swarm_notification_response() -> bytes:
        raw_data = [int(Protocol.Message.REGISTER_SWARM_NOTIFICATION_RESPONSE)]
        return bytes(raw_data)
    
    @staticmethod
    def parse_set_drone_target_position_request(raw_data: bytes) -> dict[int, Position]:
        result = dict()
        if raw_data[0] == int(Protocol.Message.SET_DRONE_TARGET_POSITION_REQUEST):
            print(int(raw_data[1]))
            for c in range(int(raw_data[1])):
                drone = Protocol.extract_uint16(raw_data[2 + c * 18 :])
                x = Protocol.extract_uint32(raw_data[4 + c * 18 :]) / 10000.0
                y = Protocol.extract_uint32(raw_data[8 + c * 18 :]) / 10000.0
                z = Protocol.extract_uint32(raw_data[12 + c * 18 :]) / 10000.0
                yaw = Protocol.extract_uint32(raw_data[16 + c * 18 :]) / 10000.0

                result[drone] = Protocol.Position(x, y, z, yaw)

            return result
        else:
            raise(Exception('Tried to parse something as a SetDroneTargetPositionRequest which isnt one!'))
        
    @staticmethod
    def parse_drone_operations_request(raw_data: bytes) -> dict[int, DroneOperation]:
        result = dict()
        if raw_data[0] == int(Protocol.Message.DRONE_OPERATIONS_REQUEST):
            for c in range(int(raw_data[1])):
                drone = Protocol.extract_uint16(raw_data[2 + c * 3 :])
                operation = Protocol.extract_uint8(raw_data[4 + c * 3 :])

                result[drone] = Protocol.DroneOperation(operation)

            return result
        else:
            raise(Exception('Tried to parse something as a DroneOperationsRequest which isnt one!'))

class DroneSwarmInterfaceServer():
    def __init__(self, port: int, host: str, initial_positions: dict[int, Protocol.Position]):
        try:
            self.port = port
            self.host = host
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.bind((self.host, self.port))
            print('Server opened on ', end='')
            print(self.host, end=':')
            print(self.port)
            self.recv_threads = []
            self.notification_threads = [] 
            
            #Save the floor height as 0
            self.floor_height = min({v.z for k,v in initial_positions.items()})

            self.drone_operations : dict[int, Protocol.DroneOperation] = dict()
            self.drone_states : dict[int, Protocol.DroneState] = dict()
            self.drone_positions : dict[int, Protocol.Position] = dict()
            self.drone_targets : dict[int, Protocol.Position] = dict()

            for drone, pos in initial_positions.items(): 
                self.drone_positions[drone] = pos
                self.drone_targets[drone] = pos
                self.drone_states[drone] = Protocol.DroneState.DRONE_IDLE
                self.drone_operations[drone] = Protocol.DroneOperation.DRONE_OPERATION_NONE
            self.swarm_state = Protocol.SwarmState.SWARM_IDLE

            self.send_mutexes : dict[socket.socket, threading.Lock] = dict()
            self.lock = threading.Lock()

            self.listen_thread = threading.Thread(target=self.handle_listen)
            self.listen_thread.start()
        except:
            print('Exception when trying to create the server!')

    def handle_listen(self):
        try:
            print('listening...')
            while True:
                self.socket.listen(4)
                connection, address = self.socket.accept()
                print('Accepted connection from ' + str(address))
                recv_thread = threading.Thread(target=self.handle_recv, args=(connection,))
                self.recv_threads.append(recv_thread)
                recv_thread.start()
        except:
            print('Exception in listen thread')
            return

    def handle_recv(self, connection: socket.socket):
        stop = False
        while not stop:
            try:
                raw_data = connection.recv(1024)
                if not raw_data:
                    stop = True
                else:
                    message_type = raw_data[0]

                    if message_type == int(Protocol.Message.REGISTER_SWARM_NOTIFICATION_REQUEST):
                        print('Received RegisterSwarmNotificationRequest')
                        interval = int(raw_data[2]) << 8 | int(raw_data[1])
                        if not connection in self.send_mutexes:
                            self.send_mutexes[connection] = threading.Lock()
                        
                        with self.send_mutexes[connection]:
                            print('Sending response...')
                            connection.send(Protocol.create_register_swarm_notification_response())
                        notification_thread = threading.Thread(target=self.handle_notification, args=(connection, self.send_mutexes[connection], interval,))
                        self.notification_threads.append(notification_thread)
                        print('Starting new notification thread...')
                        notification_thread.start()
                    elif message_type == int(Protocol.Message.SET_DRONE_TARGET_POSITION_REQUEST):
                        drone_targets = Protocol.parse_set_drone_target_position_request(raw_data)
                        print('Received new SetDroneTargetPositionRequest: ' + str(drone_targets))

                        with self.lock:
                            self.drone_targets = drone_targets
                    elif message_type == int(Protocol.Message.DRONE_OPERATIONS_REQUEST):
                        drone_operations = Protocol.parse_drone_operations_request(raw_data)
                        print('Received new DroneOperationsRequest: ' + str(drone_operations))
                        
                        with self.lock:
                            self.drone_operations = drone_operations
                    else:
                        print('Received unexpected message: ' + str(message_type))
            except:
                print('Connection closed by remote host')
                connection.close()
                stop = True

    def handle_notification(self, connection: socket.socket, mutex: threading.Lock, interval: int):
        while True:
            with mutex:
                with self.lock:
                    floor_adjusted_positions = {k: Protocol.Position(v.x, v.y, v.z-self.floor_height, v.yaw) for k,v in self.drone_positions.items()}
                    try:
                        connection.send(Protocol.create_swarm_notification_message(floor_adjusted_positions, self.drone_targets, self.drone_states, self.drone_operations, self.swarm_state))
                    except:
                        print('Notification thread stopped (connection closed)')
                        return
            time.sleep(interval / 1000.0)

