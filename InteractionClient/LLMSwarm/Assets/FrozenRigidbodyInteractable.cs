using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class FrozenRigidbodyInteractable : MonoBehaviour
{
     // Start is called before the first frame update
     void Start()
     {
        
     }

     public void onSelected()
     {
          
     }
     public void onUnselected()
     {
          
     }

     // Update is called once per frame
     void Update()
     {
          Rigidbody r = GetComponentInChildren<Rigidbody>();
          r.constraints = RigidbodyConstraints.FreezeAll;
     }
}
