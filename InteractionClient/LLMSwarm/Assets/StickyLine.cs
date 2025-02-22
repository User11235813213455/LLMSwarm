using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[ExecuteAlways]
public class StickyLine : MonoBehaviour
{
     public GameObject start;
     public GameObject end;

     public void Start()
     {
        
     }

     void Update()
     {
          LineRenderer lineRenderer = GetComponent<LineRenderer>();
          if(lineRenderer.useWorldSpace)
          {
               lineRenderer.SetPosition(0, start.transform.position);
               lineRenderer.SetPosition(1, end.transform.position);
          }
          else
          {
               lineRenderer.SetPosition(0, start.transform.localPosition);
               lineRenderer.SetPosition(1, end.transform.localPosition);
          }
     }
}
