using System;
using System.Net;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;


public class Position
{
     public Position(double pX, double pY, double pZ, double pYaw)
     {
          X = pX;
          Y = pY;
          Z = pZ;
          Yaw = pYaw;
     }
     public Position(Vector3 pVector)
     {
          X = pVector.x;
          Y = pVector.y;
          Z = pVector.z;
          Yaw = 0.0;
     }

     public override string ToString()
     {
          return "(x=" + X.ToString() + ", y=" + Y.ToString() + ", z=" + Z.ToString() + ")";
     }

     public double X { get; set; }
     public double Y { get; set; }
     public double Z { get; set; }
     public double Yaw { get; set; }

     public Vector3 toVector()
     {
          return new Vector3((float)X, (float)Y, (float)Z);
     }
}
public enum ProtocolMessageID : byte
{
     /**
      * @brief Requests cyclic notifications about the current swarm state (drone positions, drone targets, drone states, drone operations, swarm state)
      */
     REGISTER_SWARM_NOTIFICATION_REQUEST = 1,

     /**
      * @brief Response to a REGISTER_SWARM_NOTIFICATION_REQUEST to confirm the reception
      */
     REGISTER_SWARM_NOTIFICATION_RESPONSE,

     /**
      * @brief A notification message containing information about the current swarm state (drone positions, drone targets, drone states, drone operations, swarm state)
      */
     SWARM_STATE_NOTIFICATION_MESSAGE,

     /**
      * @brief A request to change the drone target coordinates
      */
     SET_DRONE_TARGET_POSITION_REQUEST,

     /**
      * @brief A response to a SET_DRONE_TARGET_POSITION_REQUEST to confirm reception
      * 
      */
     SET_DRONE_TARGET_POSITION_RESPONSE,

     /**
      * @brief A request to change the swarm operation (e.g. in order to start takeoff, landing, fast stop, movement)
      * 
      */
     SWARM_OPERATIONS_REQUEST,

     /**
      * @brief A response to a SWARM_OPERATIONS_REQUEST to confirm the reception
      * 
      */
     SWARM_OPERATIONS_RESPONSE,

     /**
      * @brief A request to change the operation of the drones
      * 
      */
     DRONE_OPERATIONS_REQUEST,

     /**
      * @brief A response to a DRONE_OPERATIONS_REQUEST confirming the reception
      * 
      */
     DRONE_OPERATIONS_RESPONSE
};

public enum DroneOperation : byte
{
     /**
      * @brief Singnals that the drone shall not start any operation currently 
      * 
      */
     DRONE_OPERATION_NONE = 0,

     /**
      * @brief Take off operation
      * 
      */
     DRONE_OPERATION_TAKE_OFF = 1,

     /**
      * @brief Landing operation
      * 
      */
     DRONE_OPERATION_LAND = 2,

     /**
      * @brief "Fast stop" -> Stop immediately and fall to the ground
      * 
      */
     DRONE_OPERATION_FAST_STOP = 3,

     /**
      * @brief Move operation -> Move the drone the its target coordinates
      * 
      */
     DRONE_OPERATION_MOVE = 4,
};
public enum SwarmState : byte
{
     /**
      * @brief Swarm is idle at the floor
      */
     SWARM_IDLE = 0,

     /**
      * @brief Swarm is currently taking off
      */
     SWARM_TAKING_OFF,

     /**
      * @brief Swarm is hovering
      */
     SWARM_HOVERING,

     /**
      * @brief Swarm is moving to another position
      */
     SWARM_MOVING,

     /**
      * @brief Swarm is landing
      */
     SWARM_LANDING,

     /**
      * @brief Swarm is fast stopping (will turn off and drop to the floor)
      */
     SWARM_STOPPING
};
public enum DroneState : byte
{
     /**
      * @brief The drone is idle on the floor
      */
     DRONE_IDLE = 0,

     /**
      * @brief The drone is currently taking off
      */
     DRONE_TAKING_OFF,

     /**
      * @brief The drone is currently hovering in the air without moving
      */
     DRONE_HOVERING,

     /**
      * @brief The drone is currently moving to its target position
      */
     DRONE_MOVING,

     /**
      * @brief The drone is currently landing
      */
     DRONE_LANDING,

     /**
      * @brief The drone is currently stopping
      */
     DRONE_STOPPING
};
public enum SwarmOperation : byte
{
     /**
      * @brief Takeoff operation of the swarm; All drones take off and fly to their target coordinates
      */
     SWARM_OPERATION_TAKEOFF = 0,

     /**
      * @brief Landing operation of the swarm; All drones land parallel
      */
     SWARM_OPERATION_LAND,

     /**
      * @brief Starts a swarm movement; All drones start moving to their target coordinates
      */
     SWARM_OPERATION_MOVE,

     /**
      * @brief Stops the swarm and lets the drones fall down
      * 
      */
     SWARM_OPERATION_FAST_STOP
};

public abstract class Message
{
     public List<byte> getBytes()
     {
          return intToByteList(getID()).Concat(getContent()).ToList();
     }
     public abstract List<byte> getContent();
     public abstract byte getID();
     public abstract UInt32 parse(List<byte> pSerializedData);

     public static List<byte> intToByteList(byte pValue)
     {
          return new List<byte>(new byte[] { pValue });
     }
     public static List<byte> intToByteList(UInt16 pValue)
     {
          byte[] intBytes = BitConverter.GetBytes(pValue);
          if (!BitConverter.IsLittleEndian)
               Array.Reverse(intBytes);
          byte[] result = intBytes;
          return new List<byte>(intBytes);
     }
     public static List<byte> intToByteList(UInt32 pValue)
     {
          byte[] intBytes = BitConverter.GetBytes(pValue);
          if (!BitConverter.IsLittleEndian)
               Array.Reverse(intBytes);
          byte[] result = intBytes;
          return new List<byte>(intBytes);
     }
     public static List<byte> intToByteList(Int16 pValue)
     {
          byte[] intBytes = BitConverter.GetBytes(pValue);
          if (!BitConverter.IsLittleEndian)
               Array.Reverse(intBytes);
          byte[] result = intBytes;
          return new List<byte>(intBytes);
     }
     public static List<byte> intToByteList(Int32 pValue)
     {
          byte[] intBytes = BitConverter.GetBytes(pValue);
          if (!BitConverter.IsLittleEndian)
               Array.Reverse(intBytes);
          byte[] result = intBytes;
          return new List<byte>(intBytes);
     }
     public static List<byte> positionMappingToByteList(Dictionary<UInt16, Position> pContent)
     {
          List<byte> result = intToByteList((byte)pContent.Count);
          foreach (KeyValuePair<UInt16, Position> kvp in pContent)
          {
               Int32 x = Convert.ToInt32(Math.Floor(kvp.Value.X * 10000.0));
               Int32 y = Convert.ToInt32(Math.Floor(kvp.Value.Y * 10000.0));
               Int32 z = Convert.ToInt32(Math.Floor(kvp.Value.Z * 10000.0));
               Int32 yaw = Convert.ToInt32(Math.Floor(kvp.Value.Yaw * 10000.0));

               result = result.Concat(intToByteList(kvp.Key)).Concat(intToByteList(x)).Concat(intToByteList(y)).Concat(intToByteList(z)).Concat(intToByteList(yaw)).ToList();
          }

          return result;
     }
     public static byte scanUint8(List<byte> pContent)
     {
          return pContent.First();
     }
     public static Int16 scanInt16(List<byte> pContent)
     {
          Int16 result = BitConverter.ToInt16(pContent.ToArray());
          return result;
     }
     public static Int32 scanInt32(List<byte> pContent)
     {
          Int32 result = BitConverter.ToInt32(pContent.ToArray());
          return result;
     }
     public static Dictionary<UInt16, Position> scanPositionMapping(List<byte> pContent, out UInt32 pLength)
     {
          Dictionary<UInt16, Position> result = new Dictionary<UInt16, Position>();
          byte length = scanUint8(pContent);

          for (byte i = 0; i < length; i++)
          {
               result[(UInt16)scanInt16(pContent.Skip(1 + i * 18).ToList())] = new Position(
                    scanInt32(pContent.Skip(3 + i * 18).ToList()) / 10000.0,
                    scanInt32(pContent.Skip(7 + i * 18).ToList()) / 10000.0,
                    scanInt32(pContent.Skip(11 + i * 18).ToList()) / 10000.0,
                    scanInt32(pContent.Skip(15 + i * 18).ToList()) / 10000.0);
          }
          pLength = (UInt32)(1 + 18 * length);
          return result;
     }
     public static Dictionary<UInt16, DroneState> scanDroneStateMapping(List<byte> pContent, out UInt32 pLength)
     {
          Dictionary<UInt16, DroneState> result = new Dictionary<UInt16, DroneState>();
          byte length = scanUint8(pContent);

          for (byte i = 0; i < length; i++)
          {
               result[(UInt16)scanInt16(pContent.Skip(1 + i * 3).ToList())] = (DroneState)scanUint8(pContent.Skip(3 + i * 3).ToList());
          }
          pLength = (UInt32)(1 + 3 * length);
          return result;
     }
     public static Dictionary<UInt16, DroneOperation> scanDroneOperationMapping(List<byte> pContent, out UInt32 pLength)
     {
          Dictionary<UInt16, DroneOperation> result = new Dictionary<UInt16, DroneOperation>();
          byte length = scanUint8(pContent);

          for (byte i = 0; i < length; i++)
          {
               result[(UInt16)scanInt16(pContent.Skip(1 + i * 3).ToList())] = (DroneOperation)scanUint8(pContent.Skip(3 + i * 3).ToList());
          }
          pLength = (UInt32)(1 + 3 * length);
          return result;
     }
}

public class RegisterSwarmStateNotificationRequest : Message
{
     /**
      * @brief Constructs a new RegisterSwarmNotificationRequest 
      * 
      * @param pInterval The interval in ms in which SwarmStateNotifications shall be sent by the receiver of this message
      */
     public RegisterSwarmStateNotificationRequest(UInt16 pInterval)
     {
          Interval = pInterval;
     }

     /**
      * @brief Constructs a RegisterSwarmStateNotificationRequest by serialized data
      * 
      * @param pSerializedData 
      */
     public RegisterSwarmStateNotificationRequest(List<byte> pSerializedData)
     {
          parse(pSerializedData);
     }

     public override List<byte> getContent()
     {
          return Message.intToByteList(this.Interval);
     }
     public override UInt32 parse(List<byte> pContent)
     {
          Interval = (UInt16)Message.scanInt16(pContent);
          return 3;
     }
     public override byte getID()
     {
          return (byte)ProtocolMessageID.REGISTER_SWARM_NOTIFICATION_REQUEST;
     }

     public UInt16 Interval { get; set; }
};

public class RegisterSwarmStateNotificiationResponse : Message
{
     public override List<byte> getContent()
     {
          return new List<byte>();
     }
     public override UInt32 parse(List<byte> pContent)
     {
          return 1;
     }
     public override byte getID()
     {
          return (byte)ProtocolMessageID.REGISTER_SWARM_NOTIFICATION_REQUEST;
     }
};

public class SwarmStateNotificationMessage : Message
{
     public SwarmStateNotificationMessage(List<Byte> pSerializedData, out UInt32 pParsedLength)
     {
          pParsedLength = parse(pSerializedData);
     }

     public override List<byte> getContent()
     {
          /*Leave empty; We don't need this function*/
          return new List<byte>();
     }
     public override UInt32 parse(List<byte> pContent)
     {
          UInt32 scanned = 1;
          UInt32 add;
          DronePositions = Message.scanPositionMapping(pContent.Skip((int)scanned).ToList(), out add);
          scanned += add;
          DroneTargets = Message.scanPositionMapping(pContent.Skip((int)scanned).ToList(), out add);
          scanned += add;
          DroneStates = Message.scanDroneStateMapping(pContent.Skip((int)scanned).ToList(), out add);
          scanned += add;
          DroneOperations = Message.scanDroneOperationMapping(pContent.Skip((int)scanned).ToList(), out add);
          scanned += add;
          SwarmState = (SwarmState)Message.scanUint8(pContent.Skip((int)scanned).ToList());

          return scanned + 1;
     }
     public override byte getID()
     {
          return (byte)ProtocolMessageID.SWARM_STATE_NOTIFICATION_MESSAGE;
     }

     public Dictionary<UInt16, Position> DronePositions { get; set; }
     public Dictionary<UInt16, Position> DroneTargets { get; set; }
     public Dictionary<UInt16, DroneState> DroneStates { get; set; }
     public Dictionary<UInt16, DroneOperation> DroneOperations { get; set; }
     public SwarmState SwarmState { get; set; }
};


public class SetDroneTargetPositionsRequest : Message
{
     public SetDroneTargetPositionsRequest(Dictionary<UInt16, Position> pTargets)
     {
          Targets = pTargets;
     }

     public override List<byte> getContent()
     {
          return Message.positionMappingToByteList(Targets);
     }
     public override UInt32 parse(List<byte> pContent)
     {
          /*Not needed*/
          return 1;
     }
     public override byte getID()
     {
          return (byte)ProtocolMessageID.SET_DRONE_TARGET_POSITION_REQUEST;
     }
     public Dictionary<UInt16, Position> Targets { get; set; }
};
public class SetDroneTargetPositionsResponse : Message
{
     public override List<byte> getContent()
     {
          return new List<byte>();
     }
     public override UInt32 parse(List<byte> pContent)
     {
          /*Not needed*/
          return 1;
     }
     public override byte getID()
     {
          return (byte)ProtocolMessageID.SET_DRONE_TARGET_POSITION_RESPONSE;
     }
};
public class SwarmOperationRequest : Message
{
     public SwarmOperationRequest(SwarmOperation pOperation)
     {
          Operation = pOperation;
     }
     public SwarmOperationRequest(List<byte> pSerializedData)
     {
          parse(pSerializedData);
     }

     public override List<byte> getContent()
     {
          return intToByteList((byte)Operation);
     }
     public override UInt32 parse(List<byte> pContent)
     {
          /*Not needed*/
          return 1;
     }
     public override byte getID()
     {
          return (byte)ProtocolMessageID.SWARM_OPERATIONS_REQUEST;
     }
     public SwarmOperation Operation { get; set; }
};
public class SwarmOperationResponse : Message
{
     public override List<byte> getContent()
     {
          return new List<byte>();
     }
     public override UInt32 parse(List<byte> pContent)
     {
          /*Not needed*/
          return 1;
     }
     public override byte getID()
     {
          return (byte)ProtocolMessageID.SWARM_OPERATIONS_RESPONSE;
     }
};