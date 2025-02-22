using System;
using UnityEngine;
using UnityEngine.InputSystem;

public class DroneTarget : MonoBehaviour
{
     public GameObject marker;
     public Color selectedColor;
     public Color inactiveColor;
     public bool selected;
     public DateTime selectionTime;
     public InputActionReference xzInput;
     public InputActionReference yInput;
     public Vector3 targetChangeThreshold;
     public GameObject xzTransformParent;
     public AnimationCurve xzCurve;
     public AnimationCurve accelerationCurve;
     public DateTime joystickActionStartXZ;
     public DateTime joystickActionStartY;
     public float speed;
     public Drone drone;
     public Snapper snapper;
     public Boundary boundary;
     public Vector3 SnappedTarget;

     public void toggleSelection()
     {
          selected = !selected;
          selectionTime = DateTime.Now;
     }
     void Start()
     {

     }

     private void updateOptics()
     {
          Renderer renderer = marker.GetComponent<Renderer>();
          if (selected)
          {
               renderer.material.color = selectedColor;
          }
          else
          {
               renderer.material.color = inactiveColor;
          }
          transform.position = marker.transform.position;
     }
     private void updateXYZInput()
     {
          Vector2 joystickXZ = xzInput.action.ReadValue<Vector2>();

          if (joystickXZ.x != 0 || joystickXZ.y != 0)
          {
               if (joystickActionStartXZ == DateTime.MinValue)
               {
                    joystickActionStartXZ = DateTime.Now;
               }
          }
          else
          {
               joystickActionStartXZ = DateTime.MinValue;
          }

          Vector2 joystickY = yInput.action.ReadValue<Vector2>();

          if (joystickY.y != 0)
          {
               if (joystickActionStartY == DateTime.MinValue)
               {
                    joystickActionStartY = DateTime.Now;
               }
          }
          else
          {
               joystickActionStartY = DateTime.MinValue;
          }

          Vector2 acc = new Vector2(joystickActionStartXZ > DateTime.MinValue ? accelerationCurve.Evaluate((float)(DateTime.Now - joystickActionStartXZ).TotalSeconds) : 0.0f,
               joystickActionStartY > DateTime.MinValue ? accelerationCurve.Evaluate((float)(DateTime.Now - joystickActionStartY).TotalSeconds) : 0.0f);

          float ySelect = Math.Sign(joystickY.y) * xzCurve.Evaluate(Math.Abs(joystickY.y)) * acc.y;
          joystickXZ = new Vector2(Math.Sign(joystickXZ.x) * xzCurve.Evaluate(Math.Abs(joystickXZ.x)) * acc.x, Math.Sign(joystickXZ.y) * xzCurve.Evaluate(Math.Abs(joystickXZ.y)) * acc.x);

          Vector3 dir = new Vector3(joystickXZ.x, ySelect, joystickXZ.y);
          dir.x = (dir.x > targetChangeThreshold.x || dir.x < -targetChangeThreshold.x) ? dir.x : 0;
          dir.y = (dir.y > targetChangeThreshold.y || dir.y < -targetChangeThreshold.y) ? dir.y : 0;
          dir.z = (dir.z > targetChangeThreshold.z || dir.z < -targetChangeThreshold.z) ? dir.z : 0;

          float yRotation = xzTransformParent.transform.eulerAngles.y;
          dir = Quaternion.Euler(0, yRotation, 0) * dir;
          if (selected)
          {
               transform.localPosition += dir * (Time.deltaTime * speed);
               if(dir.magnitude > 0)
               {
                    drone.currentWaypoint = null;
               }
          }
          if(dir.magnitude == 0)
          {
               updateSnap();
          }
     }
     public void updateClamp()
     {
          Vector3 clampedTarget = boundary.clamp(transform.localPosition);
          if(clampedTarget != transform.localPosition)
          {
               boundary.showFor(500);
          }
          transform.localPosition = clampedTarget;
     }
     public void updateSnap()
     {
          SnappedTarget = snapper.snap(transform.localPosition);
     }

     void Update()
     {
          updateOptics();
          if(drone != null)
          {
               updateXYZInput();
               updateClamp();
          }
          
     }
}
