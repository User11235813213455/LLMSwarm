using System;
using System.Collections.Generic;
using System.Linq;
using TMPro;
using UnityEngine;

public class Drone : MonoBehaviour
{
     public TextMeshProUGUI nameField;
     public Color lineColorActive;
     public Color lineColorInactive;
     public LineRenderer lineRenderer;
     public GameObject positionMarker;
     public DroneTarget target;
     public DroneState droneState;
     public DroneOperation droneOperation;
     public DeletableWaypoint currentWaypoint = null;

     public DateTime SelectionTime
     {
          get { return target.selectionTime; }
     }

     public bool Selected
     {
          get { return target.selected; }
          set { target.selected = value; }
     }
     public Vector3 Target
     {
          get { return target.SnappedTarget; }
          set 
          { 
               target.transform.localPosition = value; 
               target.updateSnap(); 
          }
     }
     public Vector3 RawTarget
     {
          get { return target.transform.localPosition; }
          set
          {
               target.transform.localPosition = value;
               target.updateSnap();
          }
     }
     public Vector3 Position
     {
          get { return positionMarker.transform.localPosition; }
          set { positionMarker.transform.localPosition = value; }
     }

     public void Start()
     {
          target.drone = this;
     }

     public void onWaypointActivated(DeletableWaypoint pWaypoint)
     {
          currentWaypoint = pWaypoint;
          Target = pWaypoint.Position;
     }

     void Update()
     {
          if(Selected)
          {
               lineRenderer.startColor = lineColorActive;
               lineRenderer.endColor = lineColorActive;
          }
          else
          {
               lineRenderer.startColor = lineColorInactive;
               lineRenderer.endColor = lineColorInactive;
          }

          if(currentWaypoint != null)
          {
               if((Position - Target).magnitude < 0.2 || !currentWaypoint.Active)
               {
                    currentWaypoint = currentWaypoint.next;
                    if(currentWaypoint != null)
                    {
                         Target = currentWaypoint.Position;
                    }
               }
          }
     }
}
