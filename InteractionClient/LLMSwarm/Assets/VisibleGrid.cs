using System.Collections.Generic;
using UnityEngine;
using System;

public class VisibleGrid : MonoBehaviour
{
     public Vector3 step;
     public Vector3 offsetMin;
     public Vector3 offsetMax;
     public float thickness;
     public Color color;
     public Material lineMaterial;
     public GameObject borderDot;
     public bool Visible;
     private List<GameObject> nodes;

     private Vector3 lastScale;

     public void Start()
     {
          nodes = new List<GameObject>();
          lastScale = Vector3.zero;
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

          nodeStart.AddComponent<Snappable>();
          nodeEnd.AddComponent<Snappable>();

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
          if (lastScale != transform.localScale)
          {
               foreach(GameObject node in nodes)
               {
                    node.SetActive(false);
                    Destroy(node);
               }
               nodes.Clear();
               float x, z;

               /*Draw XZ layer*/
               for (x = offsetMin.x; x <= transform.localScale.x - offsetMax.x; x += step.x)
               {
                    addLineRenderer(new Vector3(x, offsetMin.y, offsetMin.z), new Vector3(x, offsetMin.y, (float)Math.Floor((transform.localScale.z - offsetMax.z) / step.z) * step.z));
               }
               for (z = offsetMin.z; z <= transform.localScale.z - offsetMax.z; z += step.z)
               {
                    addLineRenderer(new Vector3(offsetMin.x, offsetMin.y, z), new Vector3((float)Math.Floor((transform.localScale.x - offsetMax.x) / step.x) * step.x, offsetMin.y, z));
               }
               if (offsetMin.y + step.y <= transform.localScale.y - offsetMax.y)
               {
                    /*We've got spikes*/
                    for (x = offsetMin.x + step.x / 2; x + step.x / 2 <= transform.localScale.x - offsetMax.x; x += step.x)
                    {
                         for (z = offsetMin.z + step.z / 2; z + step.z / 2 <= transform.localScale.z - offsetMax.z; z += step.z)
                         {
                              addLineRenderer(new Vector3(x - step.x / 2, offsetMin.y, z - step.z / 2), new Vector3(x, offsetMin.y + step.y, z));
                              addLineRenderer(new Vector3(x - step.x / 2, offsetMin.y, z + step.z / 2), new Vector3(x, offsetMin.y + step.y, z));
                              addLineRenderer(new Vector3(x + step.x / 2, offsetMin.y, z - step.z / 2), new Vector3(x, offsetMin.y + step.y, z));
                              addLineRenderer(new Vector3(x + step.x / 2, offsetMin.y, z + step.z / 2), new Vector3(x, offsetMin.y + step.y, z));
                         }
                    }
               }

               lastScale = transform.localScale;
          }
          if (Visible)
          {
               foreach (GameObject node in nodes)
               {
                    node.SetActive(true);
               }
          }
          else
          {
               foreach (GameObject node in nodes)
               {
                    node.SetActive(false);
               }
          }
     }
}
