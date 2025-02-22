using System.Collections;
using System;
using System.Collections.Generic;
using UnityEngine;

public class Spawner : MonoBehaviour
{
     public GameObject spawnObject;
     public float respawnTime;
     public float respawnMagnitude;
     
     private DateTime spawnTimer = DateTime.MinValue;
     private Vector3 initialPosition = Vector3.zero;
     private SpawnTrigger trigger;
     private GameObject originalCopy;

     public void Start()
     {
          spawnTimer = DateTime.MinValue;
          trigger = spawnObject.GetComponentInChildren<SpawnTrigger>();
          initialPosition = trigger.transform.position;
          originalCopy = GameObject.Instantiate(spawnObject, initialPosition, Quaternion.identity, transform.root);
          originalCopy.SetActive(false);
     }
     public void Update()
     {
          if (spawnTimer == DateTime.MinValue)
          {
               if(trigger == null)
               {
                    spawnTimer = DateTime.Now;
               }
               else if ((trigger.transform.position - initialPosition).magnitude > respawnMagnitude)
               {
                    spawnTimer = DateTime.Now;
               }
          }
          else
          {
               if ((DateTime.Now - spawnTimer).TotalMilliseconds > respawnTime)
               {
                    spawnObject = GameObject.Instantiate(originalCopy, initialPosition, Quaternion.identity, transform.root);
                    spawnObject.SetActive(true);
                    trigger = spawnObject.GetComponentInChildren<SpawnTrigger>();
                    spawnTimer = DateTime.MinValue;
               }
          }
     }
}