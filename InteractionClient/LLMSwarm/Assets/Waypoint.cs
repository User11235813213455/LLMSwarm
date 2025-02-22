using System;
using UnityEngine;
using UnityEngine.InputSystem;
using UnityEngine.XR.Interaction.Toolkit.Interactables;
using Utilities.Extensions;

public class Waypoint : MonoBehaviour
{
     public GameObject blinkingPreviewObject;
     public GameObject spawnObject;
     public InputActionReference placeButton;
     public InputActionReference activateButton;

     public bool blink = false;
     public bool hover = false;
     private DateTime lastBlink = DateTime.MinValue;
     public bool placePressed = false;
     public bool dbgToggleButton = false;
     public bool dbgActivatePressed;
     public bool dbgActivateReleased;
     public bool dbgSelect;
     public bool dbgToggleHover;
     private GameObject previewObject;
     private DateTime activateStart = DateTime.MinValue;
     private DateTime deleteStart = DateTime.MinValue;

     void Start()
     {
          previewObject = GameObject.Instantiate(spawnObject, transform.position, Quaternion.identity, transform);
          previewObject.SetActive(true);

          DeletableWaypoint previewDeletableWaypoint = previewObject.GetComponentInChildren<DeletableWaypoint>(true);
          previewDeletableWaypoint.Active = true;

          placeButton.action.performed += ctx => onPlaceButtonClicked();
          placeButton.action.canceled += ctx => onPlaceButtonReleased();

          activateButton.action.performed += ctx => onActivatePressed();
          activateButton.action.canceled += ctx => onActivateReleased();
     }
     public void onPlaceButtonClicked()
     {
          placePressed = true;
          if(blink)
          {
               place();
          }
          else
          {
               if(hover)
               {
                    deleteStart = DateTime.Now;
               }
          }
     }
     public void onPlaceButtonReleased()
     {
          placePressed = false;
          deleteStart = DateTime.MinValue;
     }
     public void onHoverEnter()
     {
          hover = true;
          if(placePressed && !blink)
          {
               deleteStart = DateTime.Now;
          }
     }
     public void onHoverExit()
     {
          hover = false;
          onActivateReleased();
     }

     private void place()
     {
          DeletableWaypoint previewDeletableWaypoint = previewObject.GetComponentInChildren<DeletableWaypoint>(true);
          GameObject cloneWaypoint = GameObject.Instantiate(spawnObject, transform.position, Quaternion.identity);
          XRGrabInteractable grabInteractible = cloneWaypoint.GetComponentInChildren<XRGrabInteractable>(true);
          DeletableWaypoint cloneDeletableWaypoint = cloneWaypoint.GetComponentInChildren<DeletableWaypoint>(true);

          cloneDeletableWaypoint.transform.parent.gameObject.SetActive(true);
          cloneDeletableWaypoint.gameObject.SetActive(true);
          cloneDeletableWaypoint.Active = true;
          grabInteractible.enabled = true;

          cloneDeletableWaypoint.next = previewDeletableWaypoint;
          if(previewDeletableWaypoint.prev != null)
          {
               cloneDeletableWaypoint.prev = previewDeletableWaypoint.prev;
               cloneDeletableWaypoint.prev.next = cloneDeletableWaypoint;
          }
          previewDeletableWaypoint.prev = cloneDeletableWaypoint;
     }

     public void onSelected()
     {
          blink = true;
     }
     public void onUnselected()
     {
          blink = false;
     }
     public void onActivatePressed()
     {
          if(hover)
          {
               activateStart = DateTime.Now;
          }
     }
     public void onActivateReleased()
     {
          activateStart = DateTime.MinValue;
     }

     void Update()
     {
          if ((DateTime.Now - lastBlink).TotalSeconds >= 0.6f)
          {
               blinkingPreviewObject.SetActive(false);
               lastBlink = DateTime.Now;
          }
          else if ((DateTime.Now - lastBlink).TotalSeconds >= 0.3f)
          {
               blinkingPreviewObject.SetActive(true);
          }
          else
          {
               blinkingPreviewObject.SetActive(false);
          }
          if(activateStart != DateTime.MinValue && (DateTime.Now - activateStart).TotalMilliseconds >= 750)
          {
               DeletableWaypoint previewDeletableWaypoint = previewObject.GetComponentInChildren<DeletableWaypoint>(true);
               previewDeletableWaypoint.activate();
               activateStart = DateTime.MinValue;
          }
          if(deleteStart != DateTime.MinValue && (DateTime.Now - activateStart).TotalMilliseconds >= 750)
          {
               DeletableWaypoint end = previewObject.GetComponentInChildren<DeletableWaypoint>(true);
               while(end != null)
               {
                    DeletableWaypoint p = end.prev;
                    end.delete();
                    end = p;
               }
               gameObject.SetActive(false);
               Destroy(gameObject);
          }

          if(dbgToggleButton)
          {
               if(placePressed)
               {
                    onPlaceButtonReleased();
               }
               else
               {
                    onPlaceButtonClicked();
               }
               dbgToggleButton = !dbgToggleButton;
          }
          if(dbgActivatePressed)
          {
               onActivatePressed();
               dbgActivatePressed = false;
          }
          if(dbgActivateReleased)
          {
               onActivateReleased();
               dbgActivateReleased = false;
          }
          if(dbgSelect)
          {
               if(blink)
               {
                    onUnselected();
               }
               else
               {
                    onSelected();
               }
               dbgSelect = false;
          }
          if(dbgToggleHover)
          {
               if(hover)
               {
                    onHoverExit();
               }
               else
               {
                    onHoverEnter();
               }
               dbgToggleHover = false;
          }
     }
}
