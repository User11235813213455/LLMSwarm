using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Snapper : MonoBehaviour
{
     public Vector3 snap(Vector3 pInput)
     {
          Transform minChild = null;
          float minDistance = float.MaxValue;

          foreach (Transform child in transform)
          {
               if(child.GetComponent<Snappable>() != null)
               {
                    if ((child.position - pInput).magnitude < minDistance)
                    {
                         minChild = child;
                         minDistance = (child.position - pInput).magnitude;
                    }
               }
          }

          if(minChild == null)
          {
               return pInput;
          }
          return minChild.position;
     }
     void Start()
     {
        
     }

     void Update()
     {
        
     }
}
