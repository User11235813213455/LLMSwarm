using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Collections;
using TMPro;
using Unity.XR.CoreUtils;
using UnityEngine;
using UnityEngine.InputSystem;
using OpenAI;

using OpenAIMessage = OpenAI.Chat.Message;
using OpenAI.Chat;
using OpenAI.Models;
using System.Threading.Tasks;
using System.Threading;
using UnityEngine.Rendering;
using Meta.WitAi.TTS.Utilities;
using System.Runtime.Serialization;
using System.Security.Cryptography;
using UnityEngine.Windows;
using UnityEngine.UIElements;
using System.Linq.Expressions;
using System.Linq;

public class DroneSwarm : MonoBehaviour
{
     public string[] droneNames;
     public string host;
     public int port;
     public TextMeshProUGUI logText;
     public TextMeshProUGUI dialogue;
     public GameObject observerCamera;
     public GameObject calibratedWorldOrigin;
     public GameObject calibrationCounterObject;
     public GameObject leftController;
     public TextMeshProUGUI calibrationCounterOutput;
     public GameObject dronePrefab;
     public InputActionReference multipleSelectionInput;
     public TTSSpeaker speaker;
     public InputActionReference rotationInput;
     private DateTime joystickRotationStartTime = DateTime.MinValue;
     public AnimationCurve rotationCurve;
     public AnimationCurve accelerationCurve;
     private GameObject dummyHead;
     public string debugGPTRequest;
     public bool debugGPTRequestSend;
     [TextArea(10, 10)]
     public string systemPrompt;
     [TextArea(10, 10)]
     public string introductionText;
     public string gptAnswer;
     public float thresholdRotation;
     public bool dummyMode;

     private OpenAIClient openAIClient;
     private List<OpenAIMessage> messages = null;
     private Thread llmThread = null;

     private DateTime calibrationStart = DateTime.MinValue;

     private Dictionary<UInt16, Drone> drones;

     private bool multipleSelection = false;
     private DateTime singleTime = DateTime.MinValue;

     private bool wallsInitialized = false;
     public GameObject walls;

     public TextMeshProUGUI recordButtonText;

     private Dictionary<ushort, Vector3> lastSave;

     public Position convertToUnityCoordinates(Position pPosition)
     {
          Vector3 adjustedVector = new Vector3((float)pPosition.X, (float)pPosition.Z, (float)pPosition.Y);
          return new Position(adjustedVector.x, adjustedVector.y, adjustedVector.z, 0.0);
     }
     public Position convertToLighthouseCoordinates(Position pPosition)
     {
          Vector3 adjustedVector = new Vector3((float)pPosition.X, (float)pPosition.Y, (float)pPosition.Z);
          return new Position(adjustedVector.x, adjustedVector.z, adjustedVector.y, 0.0);
     }

     private double delta;
     private InteractionClient client;

     void Start()
     {
          lastSave = new Dictionary<ushort, Vector3>();
          droneNames = new string[]
          {
               "Amelia",
               "Bonnie",
               "Charlie",
               "Daisy",
               "Elsie",
               "Freya",
               "Grace",
               "Harper",
               "Isla",
               "Jessica",
               "Kai",
               "Lily",
               "Mia",
               "Nancy",
               "Olivia",
               "Poppy",
               "Quinn",
               "Rosie",
               "Sienna",
               "Thea",
               "Una",
               "Violet",
               "Willow",
               "Xanthe",
               "Yusra",
               "Zara"
          };
          logText.text = "Starting up the client...";

          drones = new Dictionary<ushort, Drone>();
          try
          {
               client = new InteractionClient(host, port);
               logText.text = "Interaction client created...";
               openAIClient = new OpenAIClient();

               messages = new List<OpenAIMessage>();
          }
          catch (Exception ex)
          {
               logText.text = ex.Message;
               client = null;
          }

          dummyHead = new GameObject("TempReferenceHead");
          drones = new Dictionary<ushort, Drone>();
          delta = 0.0f;
          multipleSelectionInput.action.performed += ctx => updateSelectionMode();
          multipleSelectionInput.action.canceled += ctx => updateSelectionMode();
     }

     private void printSwarmState(TextMeshProUGUI pTextField,
          ConcurrentDictionary<UInt16, Position> pPositions,
          ConcurrentDictionary<UInt16, Position> pTargets,
          ConcurrentDictionary<UInt16, DroneState> pStates,
          ConcurrentDictionary<UInt16, DroneOperation> pOperations,
          SwarmState pSwarmState)
     {
          try
          {
               pTextField.text = "Positions: ";
               foreach (KeyValuePair<UInt16, Position> kvp in pPositions)
               {
                    drones[kvp.Key].Position = convertToUnityCoordinates(kvp.Value).toVector();
                    pTextField.text += kvp.Key.ToString() + ": " + drones[kvp.Key].Position.ToString() + " ";
               }
               pTextField.text += "\nRaw: ";
               foreach (KeyValuePair<UInt16, Position> kvp in pPositions)
               {
                    pTextField.text += kvp.Key.ToString() + ": " + kvp.Value.ToString() + " ";
               }
               pTextField.text += "\nTargets: ";
               foreach (KeyValuePair<UInt16, Position> kvp in pTargets)
               {
                    pTextField.text += kvp.Key.ToString() + ": " + kvp.Value.ToString() + " ";
               }
               //pTextField.text += "\nStates: ";
               foreach (KeyValuePair<UInt16, DroneState> kvp in pStates)
               {
                    drones[kvp.Key].droneState = kvp.Value;
                    //pTextField.text += kvp.Key.ToString() + ": " + kvp.Value.ToString() + " ";
               }
               //pTextField.text += "\nOperations: ";
               foreach (KeyValuePair<UInt16, DroneOperation> kvp in pOperations)
               {
                    drones[kvp.Key].droneOperation = kvp.Value;
                    //pTextField.text += kvp.Key.ToString() + ": " + kvp.Value.ToString() + " ";
               }
               //pTextField.text += "\nSwarm state: " + pSwarmState.ToString();
               pTextField.text += "\nHeadset pos.: " + observerCamera.transform.position.ToString() + " rot.: " + observerCamera.transform.rotation.eulerAngles.ToString(); ;
               pTextField.text += "\n" + "Lighthouse origin: " + calibratedWorldOrigin.transform.position.ToString() + " rot.: " + calibratedWorldOrigin.transform.rotation.eulerAngles.ToString();
          }
          catch (Exception ex)
          {
               pTextField.text += ex.Message;
          }
     }
     private enum EnqueuedOperation
     {
          ENQUEUED_OPERATION_NONE,
          ENQUEUED_OPERATION_TAKEOFF,
          ENQUEUED_OPERATION_LAND,
          ENQUEUED_OPERATION_MOVE,
          ENQUEUED_OPERATION_FAST_STOP
     }
     private EnqueuedOperation enqueuedOperation = EnqueuedOperation.ENQUEUED_OPERATION_NONE;
     public void takeoff()
     {
          enqueuedOperation = EnqueuedOperation.ENQUEUED_OPERATION_TAKEOFF;
     }
     public void land()
     {
          enqueuedOperation = EnqueuedOperation.ENQUEUED_OPERATION_LAND;
     }
     public void fastStop()
     {
          enqueuedOperation = EnqueuedOperation.ENQUEUED_OPERATION_FAST_STOP;
     }
     public void move()
     {
          enqueuedOperation = EnqueuedOperation.ENQUEUED_OPERATION_MOVE;
     }
     public void calibrate()
     {
          calibrationStart = DateTime.Now;
     }
     private void updateCalibration()
     {
          if (selectedDrones == null)
          {
               selectedDrones = new List<KeyValuePair<ushort, Drone>>();
          }
          else
          {
               selectedDrones.Clear();
          }
          foreach (var kvp in drones)
          {
               if (kvp.Value.Selected)
               {
                    selectedDrones.Add(kvp);
               }
          }
          if (calibrationStart > DateTime.MinValue)
          {
               calibrationCounterObject.SetActive(true);
               TimeSpan timeSpan = DateTime.Now - calibrationStart;
               calibrationCounterOutput.text = timeSpan.Seconds.ToString();

               if (timeSpan.Seconds >= 5)
               {
                    calibrationStart = DateTime.MinValue;
                    calibratedWorldOrigin.transform.rotation = Quaternion.Euler(0.0f, 0.0f, 0.0f);
                    calibratedWorldOrigin.transform.Rotate(new Vector3(0.0f, observerCamera.transform.rotation.eulerAngles.y + 90.0f, 0.0f), Space.Self);

                    if(selectedDrones.Count < 2)
                    {
                         throw(new Exception("Not enough drones for calibration!"));
                    }

                    Position rawPos1 = null;
                    Position rawPos2 = null;
                    Position unityPos1 = null;
                    Position unityPos2 = null;

                    foreach(var kvp in selectedDrones)
                    {
                         if(rawPos1 == null)
                         {
                              rawPos1 = new Position(drones[kvp.Key].RawTarget);
                              unityPos1 = new Position(drones[kvp.Key].Position);
                         }
                         else
                         {
                              rawPos2 = new Position(drones[kvp.Key].RawTarget);
                              unityPos2 = new Position(drones[kvp.Key].Position);
                              break;
                         }
                    }

                    /*Calculate scale*/
                    Vector3 scaling = new Vector3((float)((rawPos1.X - rawPos2.X) / (unityPos1.X - unityPos2.X)),
                         (float)((rawPos1.Y - rawPos2.Y) / (unityPos1.Y - unityPos2.Y)),
                         (float)((rawPos1.Z - rawPos2.Z) / (unityPos1.Z - unityPos2.Z)));
                    scaling = new Vector3(Math.Sign(scaling.x), Math.Sign(scaling.y), Math.Sign(scaling.z));

                    Vector3 oldScale = calibratedWorldOrigin.transform.localScale;

                    calibratedWorldOrigin.transform.localScale = new Vector3(oldScale.x * scaling.x, oldScale.y * scaling.y, oldScale.z * scaling.z);

                    /*Calculate transformation*/
                    Vector3 scaledUnityPos1 = new Vector3((float)unityPos1.X * scaling.x, (float)unityPos1.Y * scaling.y, (float)unityPos1.Z * scaling.z);
                    Vector3 rawPosVector1 = new Vector3((float)rawPos1.X, (float)rawPos1.Y, (float)rawPos1.Z);
                    Vector3 transform = rawPosVector1 - scaledUnityPos1;

                    calibratedWorldOrigin.transform.position += transform;
               }
          }
          else
          {
               calibrationCounterObject.SetActive(false);
          }
     }
     private void performEnqueuedOperation()
     {
          switch (enqueuedOperation)
          {
               case EnqueuedOperation.ENQUEUED_OPERATION_TAKEOFF:
                    {
                         client.issueTakeoff();
                         enqueuedOperation = EnqueuedOperation.ENQUEUED_OPERATION_NONE;
                    }
                    break;
               case EnqueuedOperation.ENQUEUED_OPERATION_LAND:
                    {
                         client.issueLanding();
                         enqueuedOperation = EnqueuedOperation.ENQUEUED_OPERATION_NONE;
                    }
                    break;
               case EnqueuedOperation.ENQUEUED_OPERATION_MOVE:
                    {
                         foreach (KeyValuePair<UInt16, Drone> kvp in drones)
                         {
                              client.DroneTargets.AddOrUpdate(kvp.Key, convertToLighthouseCoordinates(new Position(kvp.Value.RawTarget)), (key, oldValue) => convertToLighthouseCoordinates(new Position(kvp.Value.RawTarget)));
                         }
                         if(!dummyMode)
                         {
                              client.updateDroneTargets();
                              client.issueMovement();
                         }
                         
                         enqueuedOperation = EnqueuedOperation.ENQUEUED_OPERATION_NONE;
                    }
                    break;
               case EnqueuedOperation.ENQUEUED_OPERATION_FAST_STOP:
                    {
                         client.issueFastStop();
                         enqueuedOperation = EnqueuedOperation.ENQUEUED_OPERATION_NONE;
                    }
                    break;
               default:
                    {
                         /*Do nothing*/
                    }
                    break;
          }
     }
     private void tts(string pText)
     {
          speaker.Speak(pText);
     }
     private void updateInterfaceClient()
     {
          if (client == null)
          {
               return;
          }
          try
          {
               client.update(logText);

               switch (client.State)
               {
                    case InteractionClient.ClientState.CLIENT_INITIALIZING:
                         {

                         }
                         break;
                    case InteractionClient.ClientState.CLIENT_REGISTER_NOTIFICATION:
                         {

                         }
                         break;
                    case InteractionClient.ClientState.CLIENT_WAIT_FIRST_STATE_MESSAGE:
                         {

                         }
                         break;
                    case InteractionClient.ClientState.CLIENT_RUNNING:
                         {
                              ConcurrentDictionary<UInt16, Position> positions = client.DronePositions;
                              ConcurrentDictionary<UInt16, DroneState> states = client.DroneStates;
                              ConcurrentDictionary<UInt16, DroneOperation> operations = client.DroneOperations;
                              ConcurrentDictionary<UInt16, Position> targets = new ConcurrentDictionary<UInt16, Position>();
                              SwarmState swarmState = client.CurrentSwarmState;
                              bool targetChanged = false;
                              foreach (KeyValuePair<UInt16, Position> kvp in positions)
                              {
                                   if (!drones.ContainsKey(kvp.Key))
                                   {
                                        GameObject droneObj = GameObject.Instantiate(dronePrefab, Vector3.zero, Quaternion.identity, calibratedWorldOrigin.transform);
                                        Drone drone;
                                        droneObj.TryGetComponent<Drone>(out drone);
                                        droneObj.GetComponentInChildren<DroneTarget>().xzTransformParent = leftController;

                                        Vector3 pos = convertToUnityCoordinates(kvp.Value).toVector();
                                        drone.target.snapper = walls.GetComponent<Snapper>();
                                        drone.target.boundary = walls.GetComponent<Boundary>();
                                        drone.RawTarget = pos;
                                        drone.Position = pos;
                                        drone.droneState = states[kvp.Key];
                                        drone.droneOperation = operations[kvp.Key];
                                        drone.nameField.text = kvp.Key <= 25 ? droneNames[kvp.Key] : kvp.Key.ToString();

                                        drones.Add(kvp.Key, drone);
                                        lastSave.Add(kvp.Key, drone.RawTarget);
                                        client.DroneTargets.TryAdd(kvp.Key, kvp.Value);
                                   }
                                   else if(client.DroneTargets.ContainsKey(kvp.Key))
                                   {
                                        Position currentDroneTarget = convertToLighthouseCoordinates(new Position(drones[kvp.Key].RawTarget));
                                        if (client.DroneTargets[kvp.Key].X != currentDroneTarget.X || client.DroneTargets[kvp.Key].Y != currentDroneTarget.Y || client.DroneTargets[kvp.Key].Z != currentDroneTarget.Z)
                                        {
                                             targetChanged = true;
                                        }
                                   }
                                   
                                   targets.AddOrUpdate(kvp.Key, new Position(drones[kvp.Key].RawTarget), (key, oldValue) => new Position(drones[kvp.Key].RawTarget));
                              }
                              if(!dummyMode && targetChanged && enqueuedOperation == EnqueuedOperation.ENQUEUED_OPERATION_NONE && (swarmState == SwarmState.SWARM_HOVERING || swarmState == SwarmState.SWARM_MOVING))
                              {
                                   enqueuedOperation = EnqueuedOperation.ENQUEUED_OPERATION_MOVE;
                              }

                              printSwarmState(logText, positions, targets, states, operations, swarmState);

                              if(!wallsInitialized)
                              {
                                   Vector3 min = new Vector3(float.MaxValue, float.MaxValue, float.MaxValue);
                                   Vector3 max = new Vector3(float.MinValue, float.MinValue, float.MinValue);
                                   foreach (KeyValuePair<UInt16, Drone> kvp in drones)
                                   {
                                        min.x = kvp.Value.Position.x < min.x ? kvp.Value.Position.x : min.x;
                                        min.y = kvp.Value.Position.y < min.y ? kvp.Value.Position.y : min.y;
                                        min.z = kvp.Value.Position.z < min.z ? kvp.Value.Position.z : min.z;
                                        max.x = kvp.Value.Position.x > max.x ? kvp.Value.Position.x : max.x;
                                        max.y = kvp.Value.Position.y > max.y ? kvp.Value.Position.y : max.y;
                                        max.z = kvp.Value.Position.z > max.z ? kvp.Value.Position.z : max.z;
                                   }
                                   min.y = 1.0f;
                                   max.y = 1.7f;
                                   walls.transform.localScale = (max - min);
                                   walls.transform.localPosition = min + new Vector3(walls.transform.localScale.x / 2, walls.transform.localScale.y / 2, walls.transform.localScale.z / 2);
                                   wallsInitialized = true;

                                   systemPrompt = systemPrompt.Replace("X_MIN", min.x.ToString().Replace(",", "."))
                                             .Replace("X_MAX", max.x.ToString().Replace(",", "."))
                                             .Replace("Y_MIN", min.y.ToString().Replace(",", "."))
                                             .Replace("Y_MAX", max.y.ToString().Replace(",", "."))
                                             .Replace("Z_MIN", min.z.ToString().Replace(",", "."))
                                             .Replace("Z_MAX", max.z.ToString().Replace(",", "."));

                                   messages = new List<OpenAIMessage>
                                   {
                                        new OpenAIMessage(Role.System, systemPrompt)
                                   };
                                   
                                   tts(introductionText);
                              }

                              performEnqueuedOperation();
                         }
                         break;
                    case InteractionClient.ClientState.CLIENT_DISCONNECTED:
                         {
                              logText.text = "Client disconnected!";
                         }
                         break;
                    case InteractionClient.ClientState.CLIENT_ERROR:
                         {
                              logText.text = "Client error!";
                         }
                         break;
               }
          }
          catch (Exception ex)
          {
               logText.text = ex.ToString();
          }
     }
     [Serializable]
     public class Coordinate
     {
          public string name;
          public double x;
          public double y;
          public double z;
     }
     [Serializable]
     public class ResponseJSON
     {
          public string answer;
          public Coordinate[] coordinates;
     }

     private List<KeyValuePair<ushort, Drone>> selectedDrones = null;

     public void saveCurrentTargets()
     {
          foreach(var kvp in drones)
          {
               lastSave[kvp.Key] = kvp.Value.RawTarget;
          }
     }
     public void restoreLastTargets()
     {
          foreach (var kvp in drones)
          {
               drones[kvp.Key].RawTarget = lastSave[kvp.Key];
               drones[kvp.Key].currentWaypoint = null;
          }
     }
     public void rotatePoints(ref Vector3[] pPoints, float pRotationYDegrees)
     {
          Vector3 geometricCenter = Vector3.zero;
          var count = 0;

          foreach (var v in pPoints)
          {
               geometricCenter += v;
               count++;
          }

          if (count > 0)
          {
               geometricCenter = new Vector3(geometricCenter.x / count, geometricCenter.y / count, geometricCenter.z / count);
          }

          for(count = 0; count < pPoints.Length; count++)
          {
               pPoints[count] = Quaternion.Euler(0, pRotationYDegrees, 0) * (pPoints[count] - geometricCenter) + geometricCenter;
          }
     }
     public void resetLLMMemory()
     {
          messages.Clear();
          messages.Add(new OpenAIMessage(Role.System, systemPrompt));
     }

     public async Task updateTargetsSpeechWorker(string pStr)
     {
          recordButtonText.text = "Recorded. Waiting for answer...";

          dialogue.text += "Input: " + pStr + "\n";

          string command = pStr;

          if (selectedDrones == null)
          {
               selectedDrones = new List<KeyValuePair<ushort, Drone>>();
          }
          else
          {
               selectedDrones.Clear();
          }
          foreach (var kvp in drones)
          {
               if (kvp.Value.Selected)
               {
                    selectedDrones.Add(kvp);
               }
          }
          string msg = command + " INFO: I have " + selectedDrones.Count + " drones at the positions ";
          foreach (var kvp in selectedDrones)
          {
               string[] coords = {
                    Math.Round(kvp.Value.RawTarget.x, 2).ToString().Replace(",","."),
                    Math.Round(kvp.Value.RawTarget.y, 2).ToString().Replace(",","."),
                    Math.Round(kvp.Value.RawTarget.z, 2).ToString().Replace(",",".")
               };

               msg += droneNames[kvp.Key];
               msg += ": ";
               msg += "(x=";
               msg += coords[0];
               msg += ", y=";
               msg += coords[1];
               msg += ", z=";
               msg += coords[2];
               msg += "), ";
          }

          gptAnswer += msg + "\n----------------------------------------------";
          messages.Add(new OpenAIMessage(Role.User, msg));

          Model model = Model.GPT4o;
          var request = new ChatRequest(messages, model, maxTokens: 500);
          string result = "";

          result = await openAIClient.ChatEndpoint.GetCompletionAsync(request);/*, r =>
          {
               result = r.FirstChoice.Message.Content.ToString();
          });*/

          messages.Add(new OpenAIMessage(Role.Assistant, result));
          gptAnswer += result + "\n------------------------------------------";
          
          if(result.Contains('{'))
          {
               result = result.Substring(result.IndexOf('{'));
               if(result.Contains('}'))
               {
                    result = result.Substring(0, result.LastIndexOf('}') + 1);
               }
          }

          dialogue.text += "Assistant: ";

          try
          {
               ResponseJSON r = JsonUtility.FromJson<ResponseJSON>(result);
               speaker.Speak(r.answer);
               dialogue.text += r.answer;

               Vector3[] points = new Vector3[r.coordinates.Length];
               
               var cntr = 0;
               
               for(cntr=0; cntr<points.Length; cntr++)
               {
                    points[cntr] = new Vector3((float)r.coordinates[cntr].x, (float)r.coordinates[cntr].y, (float)r.coordinates[cntr].z);
               }

               cntr = 0;

               for(cntr=0; cntr<points.Length; cntr++)
               {
                    if (!droneNames.Contains(r.coordinates[cntr].name))
                    {
                         break;
                    }
                    int index = Array.IndexOf(droneNames, r.coordinates[cntr].name);
                    drones[(ushort)index].RawTarget = points[cntr];
                    drones[(ushort)index].currentWaypoint = null;

                    dialogue.text += drones[(ushort)index].RawTarget;
               }
               if (r.coordinates.Length != drones.Count)
               {
                    dialogue.text += " <INVALID LENGTH!> ";
               }
          }
          catch (Exception e)
          {
               dialogue.text += "<ERROR! " + e.Message + ">";
          }

          recordButtonText.text = "Voice Command";
     }
     public async void updateTargetsSpeech(string pStr)
     {
          if(messages == null)
          {
               return;
          }
          await updateTargetsSpeechWorker(pStr);
     }
     public void updateSelectionMode()
     {
          multipleSelection = !multipleSelection;

          if (multipleSelection == false)
          {
               singleTime = DateTime.Now;
          }
     }
     public void updateSelection()
     {
          if (!multipleSelection)
          {
               bool anyLater = false;
               foreach (var kvp in drones)
               {
                    if (kvp.Value.SelectionTime > singleTime)
                    {
                         anyLater = true;
                         break;
                    }
               }
               if (anyLater)
               {
                    Drone max = null;
                    DateTime maxTime = DateTime.MinValue;
                    foreach (var kvp in drones)
                    {
                         if (kvp.Value.SelectionTime > maxTime)
                         {
                              max = kvp.Value;
                              maxTime = kvp.Value.SelectionTime;
                         }
                         kvp.Value.Selected = false;
                    }

                    if (max != null)
                    {
                         max.Selected = true;
                    }
               }
          }
     }
     public void onWaypointActivated(DeletableWaypoint pWaypoint)
     {
          if (selectedDrones == null)
          {
               selectedDrones = new List<KeyValuePair<ushort, Drone>>();
          }
          else
          {
               selectedDrones.Clear();
          }
          foreach (var kvp in drones)
          {
               if (kvp.Value.Selected)
               {
                    selectedDrones.Add(kvp);
               }
          }

          foreach (var kvp in selectedDrones)
          {
               kvp.Value.onWaypointActivated(pWaypoint);
          }
     }
     public void onWaypointDeleted(DeletableWaypoint pWaypoint)
     {
          foreach (var kvp in drones)
          {
               if(kvp.Value.currentWaypoint == pWaypoint)
               {
                    kvp.Value.currentWaypoint = pWaypoint.next;
               }
          }
     }
     public void updateRotation()
     {
          if (selectedDrones == null)
          {
               selectedDrones = new List<KeyValuePair<ushort, Drone>>();
          }
          else
          {
               selectedDrones.Clear();
          }
          foreach (var kvp in drones)
          {
               if (kvp.Value.Selected)
               {
                    selectedDrones.Add(kvp);
               }
          }


          Vector2 joystickRotation = rotationInput.action.ReadValue<Vector2>();

          if(Math.Abs(joystickRotation.x) < thresholdRotation)
          {
               joystickRotation.x = 0.0f;
          }

          if (joystickRotation.x != 0)
          {
               if (joystickRotationStartTime == DateTime.MinValue)
               {
                    logText.text += "Started rotation\n";
                    joystickRotationStartTime = DateTime.Now;
               }
          }
          else
          {
               joystickRotationStartTime = DateTime.MinValue;
               logText.text += "Ended rotation\n";
          }

          float acc = joystickRotationStartTime > DateTime.MinValue ? accelerationCurve.Evaluate((float)(DateTime.Now - joystickRotationStartTime).TotalSeconds) : 0.0f;
          logText.text += "Acceleration: " + acc.ToString() + "\n";

          float rotate = Math.Sign(joystickRotation.x) * rotationCurve.Evaluate(Math.Abs(joystickRotation.x)) * acc;
          logText.text += "Rotation: " + rotate.ToString() + "\n";

          Vector3 geometricCenter = Vector3.zero;
          var count = 0;

          foreach(var kvp in selectedDrones)
          {
               geometricCenter += drones[kvp.Key].target.transform.position;
               count++;
          }

          if(count > 0)
          {
               geometricCenter = new Vector3(geometricCenter.x / count, geometricCenter.y / count, geometricCenter.z / count);
          }

          logText.text += "Geometric center: " + geometricCenter.ToString() + "\n";

          foreach(var kvp in selectedDrones)
          {
               drones[kvp.Key].target.transform.RotateAround(geometricCenter, Vector3.up, rotate);
          }
     }

     void Update()
     {
          delta += Time.deltaTime;
          if(debugGPTRequestSend)
          {
               debugGPTRequestSend = false;
               updateTargetsSpeechWorker(debugGPTRequest);
          }
          updateInterfaceClient();
          updateCalibration();
          updateSelection();
          updateRotation();

          if(dummyMode)
          {
               foreach(var kvp in client.CurrentDroneTargets)
               {
                    Vector3 currentTarget = drones[kvp.Key].RawTarget;
                    currentTarget.x = (float)kvp.Value.X;
                    currentTarget.y = (float)kvp.Value.Z;
                    currentTarget.z = (float)kvp.Value.Y;
                    drones[kvp.Key].RawTarget = currentTarget;
               }
          }
     }
}
