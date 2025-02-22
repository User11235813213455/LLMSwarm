using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class SimulatedButton : MonoBehaviour
{
     public bool click = false;
     // Start is called before the first frame update
     void Start()
     {
        
     }

     // Update is called once per frame
     void Update()
     {
          if(click)
          {
               click = false;
               gameObject.GetComponent<Button>().onClick.Invoke();
          }
     }
}
