using System;
using System.Net.Sockets;
using System.Threading;
using System.Net;
using System.Collections.Generic;
using System.Collections.Concurrent;
using System.Linq;
using TMPro;

public class InteractionClient
{
     public enum ClientState
     {
          CLIENT_NOT_INIT,
          CLIENT_INITIALIZING,
          CLIENT_REGISTER_NOTIFICATION,
          CLIENT_WAIT_FIRST_STATE_MESSAGE,
          CLIENT_RUNNING,
          CLIENT_DISCONNECTED,
          CLIENT_ERROR
     }
     public InteractionClient(string host, int port)
     {
          _receiveBufferMutex = new Mutex();
          _receiveBuffer = new List<byte>();
          _buffer = new byte[_maxBufferSize];
          DronePositions = new ConcurrentDictionary<UInt16, Position>();
          DroneTargets = new ConcurrentDictionary<UInt16, Position>();
          DroneStates = new ConcurrentDictionary<UInt16, DroneState>();
          DroneOperations = new ConcurrentDictionary<UInt16, DroneOperation>();
          CurrentDroneTargets = new ConcurrentDictionary<UInt16, Position>();
          CurrentSwarmState = SwarmState.SWARM_IDLE;
          State = ClientState.CLIENT_NOT_INIT;

          IPAddress[] addresslist = Dns.GetHostAddresses(host);
          IPAddress ip = null;
          Console.WriteLine("Host " + host + " resolved to the following IPs:");

          foreach (IPAddress theaddress in addresslist)
          {
               Console.WriteLine(theaddress.ToString());
               if (theaddress.AddressFamily == AddressFamily.InterNetwork)
               {
                    ip = theaddress;
               }
          }

          _tcpClient = new TcpClient(addresslist[0].AddressFamily);
          _tcpClient.ReceiveBufferSize = 8192;
          _tcpClient.SendBufferSize = 8192;
          _tcpClient.ReceiveTimeout = 0;

          State = ClientState.CLIENT_INITIALIZING;
          if (ip == null)
          {
               State = ClientState.CLIENT_ERROR;
          }
          else
          {
               _tcpClient.BeginConnect(ip, port, OnConnected, _tcpClient);
          }
     }
     private void OnReceive(IAsyncResult pResult)
     {
          try
          {
               int readBytes = _stream.EndRead(pResult);
               if (readBytes <= 0)
               {
                    /*Disconnected*/
                    State = ClientState.CLIENT_DISCONNECTED;
                    return;
               }
               else
               {
                    _receiveBufferMutex.WaitOne();
                    _receiveBuffer = _receiveBuffer.Concat(_buffer.Take(readBytes).ToList()).ToList();
                    _receiveBufferMutex.ReleaseMutex();
                    _stream.BeginRead(_buffer, 0, _maxBufferSize, OnReceive, null);
               }
          }
          catch (Exception)
          {
               Console.WriteLine("Error!");
               State = ClientState.CLIENT_ERROR;
          }
     }
     private void updateByStateMessage(SwarmStateNotificationMessage pMessage)
     {
          foreach (KeyValuePair<UInt16, Position> kvp in pMessage.DroneTargets)
          {
               CurrentDroneTargets[kvp.Key] = kvp.Value;
          }
          foreach (KeyValuePair<UInt16, Position> kvp in pMessage.DronePositions)
          {
               DronePositions[kvp.Key] = kvp.Value;
          }
          foreach (KeyValuePair<UInt16, DroneState> kvp in pMessage.DroneStates)
          {
               DroneStates[kvp.Key] = kvp.Value;
          }
          foreach (KeyValuePair<UInt16, DroneOperation> kvp in pMessage.DroneOperations)
          {
               DroneOperations[kvp.Key] = kvp.Value;
          }
          CurrentSwarmState = pMessage.SwarmState;
     }
     public void update(TextMeshProUGUI pText)
     {
          try
          {
               switch (State)
               {
                    case ClientState.CLIENT_INITIALIZING:
                    {
                         /*Ignore; We are still trying to connect*/
                    }
                    break;
                    case ClientState.CLIENT_REGISTER_NOTIFICATION:
                         send(new RegisterSwarmStateNotificationRequest(70));
                         State = ClientState.CLIENT_WAIT_FIRST_STATE_MESSAGE;
                         break;
                    case ClientState.CLIENT_WAIT_FIRST_STATE_MESSAGE:
                         if(_receiveBufferMutex.WaitOne(100))
                         {
                              if (_receiveBuffer.Count > 200)
                              {
                                   if (_receiveBuffer.First() == (byte)ProtocolMessageID.SWARM_STATE_NOTIFICATION_MESSAGE)
                                   {
                                        UInt32 length;
                                        SwarmStateNotificationMessage msg = new SwarmStateNotificationMessage(_receiveBuffer, out length);
                                        _receiveBuffer.RemoveRange(0, (int)length);
                                        updateByStateMessage(msg);
                                        State = ClientState.CLIENT_RUNNING;
                                   }
                              }
                              _receiveBufferMutex.ReleaseMutex();
                         }
                         break;
                    case ClientState.CLIENT_RUNNING:
                         if(_receiveBufferMutex.WaitOne(10))
                         {
                              if (_receiveBuffer.Count > 200)
                              {
                                   if (_receiveBuffer.First() == (byte)ProtocolMessageID.SWARM_STATE_NOTIFICATION_MESSAGE)
                                   {
                                        UInt32 length;
                                        SwarmStateNotificationMessage msg = new SwarmStateNotificationMessage(_receiveBuffer, out length);
                                        _receiveBuffer.RemoveRange(0, (int)length);
                                        updateByStateMessage(msg);
                                        State = ClientState.CLIENT_RUNNING;
                                   }
                              }

                              _receiveBufferMutex.ReleaseMutex();
                         }
                         break;
                    default:
                         /*Do nothing at all*/
                         break;
               }
          }
          catch(Exception exc)
          {
               pText.text = exc.Message;
          }
     }
     public void updateDroneTargets()
     {
          Dictionary<UInt16, Position> targets = new Dictionary<UInt16, Position>();
          foreach (KeyValuePair<UInt16, Position> kv in DroneTargets)
          {
               targets[kv.Key] = kv.Value;
          }
          send(new SetDroneTargetPositionsRequest(targets));
     }
     public void issueMovement()
     {
          send(new SwarmOperationRequest(SwarmOperation.SWARM_OPERATION_MOVE));
     }
     public void issueTakeoff()
     {
          send(new SwarmOperationRequest(SwarmOperation.SWARM_OPERATION_TAKEOFF));
     }
     public void issueLanding()
     {
          send(new SwarmOperationRequest(SwarmOperation.SWARM_OPERATION_LAND));
     }
     public void issueFastStop()
     {
          send(new SwarmOperationRequest(SwarmOperation.SWARM_OPERATION_FAST_STOP));
     }
     private void OnConnected(IAsyncResult pResult)
     {
          try
          {
               _tcpClient.EndConnect(pResult);
               if (_tcpClient.Connected)
               {
                    _buffer = new byte[_maxBufferSize];
                    _tcpClient.ReceiveTimeout = 100;
                    _stream = _tcpClient.GetStream();
                    _stream.BeginRead(_buffer, 0, _maxBufferSize, OnReceive, null);
                    State = ClientState.CLIENT_REGISTER_NOTIFICATION;
               }
               else
               {
                    State = ClientState.CLIENT_DISCONNECTED;
               }
          }
          catch (Exception)
          {
               State = ClientState.CLIENT_ERROR;
          }
     }
     private bool send(Message pMessage)
     {
          List<byte> data = pMessage.getBytes();
          try
          {
               _stream.WriteAsync(data.ToArray());
               return true;
          }
          catch (Exception)
          {
               return false;
          }
     }

     protected TcpClient _tcpClient;
     protected NetworkStream _stream;
     protected byte[] _buffer;
     protected const int _maxBufferSize = 8192;
     protected Mutex _receiveBufferMutex;
     private List<byte> _receiveBuffer { get; set; }
     public ClientState State { get; private set; }
     public ConcurrentDictionary<UInt16, Position> DronePositions { get; private set; }
     public ConcurrentDictionary<UInt16, Position> DroneTargets { get; private set; }
     public ConcurrentDictionary<UInt16, Position> CurrentDroneTargets { get; private set; }
     public ConcurrentDictionary<UInt16, DroneState> DroneStates { get; private set; }
     public ConcurrentDictionary<UInt16, DroneOperation> DroneOperations { get; private set; }
     public SwarmState CurrentSwarmState { get; private set; }
}