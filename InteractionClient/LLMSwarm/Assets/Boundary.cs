using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;

public class Boundary : MonoBehaviour
{
     public float thickness;
     public Color color;
     public Material lineMaterial;
     public GameObject borderDot;
     private List<GameObject> nodes;
     private DateTime showStart = DateTime.MinValue;
     private float showTime = 0.0f;

     private Vector3 lastScale;

     public Vector3 clamp(Vector3 pCoordinate)
     {
          Vector3 result = new Vector3(Math.Min(Math.Max(pCoordinate.x, transform.position.x - transform.lossyScale.x / 2), transform.position.x + transform.lossyScale.x / 2),
               Math.Min(Math.Max(pCoordinate.y, transform.position.y - transform.lossyScale.y / 2), transform.position.y + transform.lossyScale.y / 2),
               Math.Min(Math.Max(pCoordinate.z, transform.position.z - transform.lossyScale.z / 2), transform.position.z + transform.lossyScale.z / 2));
          return result;
     }

     public void showFor(uint pMilliseconds)
     {
          show(true);
          showTime = pMilliseconds;
          showStart = DateTime.Now;
     }
     public void show(bool pShow)
     {
          if(pShow)
          {
               if(nodes.Count == 0)
               {
                    lastScale = Vector3.zero;
               }
          }
          else
          {
               return;
               foreach (GameObject node in nodes)
               {
                    node.SetActive(false);
                    Destroy(node);
               }
               nodes.Clear();
          }
     }

     public void Start()
     {
          nodes = new List<GameObject>();
          lastScale = Vector3.zero;
          show(true);
     }

     private void addLineRenderer(Vector3 from, Vector3 to)
     {
          from = new Vector3(from.x * (1.0f / transform.localScale.x),
               from.y * (1.0f / transform.localScale.y),
               from.z * (1.0f / transform.localScale.z));
          to = new Vector3(to.x * (1.0f / transform.localScale.x),
               to.y * (1.0f / transform.localScale.y),
               to.z * (1.0f / transform.localScale.z));

          GameObject nodeStart = Instantiate(borderDot, Vector3.zero, Quaternion.identity, transform);
          nodeStart.transform.localScale = new Vector3((1.0f / transform.localScale.x) * nodeStart.transform.localScale.x,
               (1.0f / transform.localScale.y) * nodeStart.transform.localScale.y,
               (1.0f / transform.localScale.z) * nodeStart.transform.localScale.z);

          nodeStart.transform.localPosition = from - new Vector3(0.5f, 0.5f, 0.5f);
          nodes.Add(nodeStart);
          GameObject nodeEnd = Instantiate(borderDot, Vector3.zero, Quaternion.identity, transform);
          nodeEnd.transform.localScale = new Vector3((1.0f / transform.localScale.x) * nodeEnd.transform.localScale.x,
               (1.0f / transform.localScale.y) * nodeEnd.transform.localScale.y,
               (1.0f / transform.localScale.z) * nodeEnd.transform.localScale.z);
          nodeEnd.transform.localPosition = to - new Vector3(0.5f, 0.5f, 0.5f);
          nodes.Add(nodeEnd);

          LineRenderer lr = nodeStart.AddComponent<LineRenderer>();
          lr.startWidth = thickness;
          lr.endWidth = thickness;
          lr.startColor = color;
          lr.endColor = color;
          lr.material = lineMaterial;
          lr.positionCount = 2;
          lr.SetPositions(new Vector3[] { Vector3.zero, to - from });

          StickyLine sticky = nodeStart.AddComponent<StickyLine>();
          sticky.start = nodeStart;
          sticky.end = nodeEnd;
     }

     void Update()
     {
          if(showStart != DateTime.MinValue)
          {
               if((DateTime.Now - showStart).TotalMilliseconds > showTime)
               {
                    showStart = DateTime.MinValue;
                    show(false);
               }
          }
          if (lastScale != transform.localScale)
          {
               foreach (GameObject node in nodes)
               {
                    node.SetActive(false);
                    Destroy(node);
               }
               nodes.Clear();

               float x;


               for(x = 0.0f; x <= transform.localScale.x; x+=transform.localScale.x)
               {
                    addLineRenderer(new Vector3(x, 0.0f, 0.0f), new Vector3(x, 0.0f, transform.localScale.z));
                    addLineRenderer(new Vector3(x, 0.0f, 0.0f), new Vector3(x, transform.localScale.y, 0.0f));
                    addLineRenderer(new Vector3(x, 0.0f, transform.localScale.z), new Vector3(x, transform.localScale.y, transform.localScale.z));
                    addLineRenderer(new Vector3(x, transform.localScale.y, 0.0f), new Vector3(x, transform.localScale.y, transform.localScale.z));
               }
               addLineRenderer(new Vector3(0.0f, 0.0f, 0.0f), new Vector3(transform.localScale.x, 0.0f, 0.0f));
               addLineRenderer(new Vector3(0.0f, transform.localScale.y, 0.0f), new Vector3(transform.localScale.x, transform.localScale.y, 0.0f));
               addLineRenderer(new Vector3(0.0f, 0.0f, transform.localScale.z), new Vector3(transform.localScale.x, 0.0f, transform.localScale.z));
               addLineRenderer(new Vector3(0.0f, transform.localScale.y, transform.localScale.z), new Vector3(transform.localScale.x, transform.localScale.y, transform.localScale.z));

               lastScale = transform.localScale;
          }
     }
}
