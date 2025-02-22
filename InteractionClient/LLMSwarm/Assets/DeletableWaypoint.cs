using System;
using UnityEngine;
using UnityEngine.InputSystem;

public class DeletableWaypoint : MonoBehaviour
{
     public DroneSwarm swarm;
     public InputActionReference deleteButton;
     public InputActionReference activateButton;
     private bool activeHover = false;
     private bool deletePressed = false;
     private bool activatePressed = false;
     public DeletableWaypoint prev = null;
     public DeletableWaypoint next = null;
     private DateTime deleteTime = DateTime.MinValue;
     private DateTime activateTime = DateTime.MinValue;
     private StickyLine stickyLine;

     void Start()
     {
          deleteButton.action.performed += ctx => setDeletePressed();
          deleteButton.action.canceled += ctx => setDeleteReleased();
          activateButton.action.performed += ctx => setActivatePressed();
          activateButton.action.canceled += ctx => setActivateReleased();
          stickyLine = GetComponent<StickyLine>();
     }

     public void setDeletePressed()
     {
          deletePressed = true;
     }
     public void setDeleteReleased()
     {
          deletePressed = false;
     }
     public void toggleSelection()
     {
          activeHover = !activeHover;
     }
     public void setActivatePressed()
     {
          activatePressed = true;
     }
     public void setActivateReleased()
     {
          activatePressed = false;
     }
     public bool Active 
     { 
          get { return transform.parent.GetComponent<DroneTarget>().selected; } 
          set { transform.parent.GetComponent<DroneTarget>().selected = value; } 
     }
     public Vector3 Position
     {
          get { return gameObject.transform.position; }
     }
     public void activate()
     {
          swarm.onWaypointActivated(this);
          activateTime = DateTime.MinValue;
     }
     public void delete()
     {
          if (prev != null)
          {
               prev.next = next;
               if(next != null)
               {
                    next.prev = prev;
               }
          }
          else if(next != null)
          {
               next.prev = null;
          }

          swarm.onWaypointDeleted(this);

          GameObject obj = transform.parent.gameObject;
          obj.SetActive(false);
          Destroy(obj);
     }
     void Update()
     {
          if(activeHover)
          {
               if(deletePressed)
               {
                    if (deleteTime == DateTime.MinValue)
                    {
                         deleteTime = DateTime.Now;
                    }
                    else if ((DateTime.Now - deleteTime).TotalMilliseconds >= 750)
                    {
                         delete();
                    }
               }
               else
               {
                    deleteTime = DateTime.MinValue;
               }
               if(activatePressed)
               {
                    if (activateTime == DateTime.MinValue)
                    {
                         activateTime = DateTime.Now;
                    }
                    else if ((DateTime.Now - activateTime).TotalMilliseconds >= 750)
                    {
                         activate();
                    }
               }
               else
               {
                    if(activateTime != DateTime.MinValue && (DateTime.Now - activateTime).TotalMilliseconds < 750)
                    {
                         DroneTarget t = transform.parent.gameObject.GetComponent<DroneTarget>();
                         t.selected = !t.selected;
                    }
                    activateTime = DateTime.MinValue;
               }
          }
          else
          {
               deleteTime = DateTime.MinValue;
               activateTime = DateTime.MinValue;
          }

          if(prev != null)
          {
               stickyLine.start = prev.gameObject;
          }
          else
          {
               stickyLine.start = gameObject;
          }
          stickyLine.end = gameObject;
     }
}
