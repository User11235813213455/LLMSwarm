using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;

public class ColorChanger : MonoBehaviour
{
    public Color color;
    // Start is called before the first frame update
    void Start()
    {
    }

     public void setColor(Color pColor)
     {
          color = pColor;
     }

     public void setWhite()
     {
          color = Color.white;
     }
     public void setRed()
     {
          color = Color.red; 
     }


    // Update is called once per frame
    void Update()
    {
          TextMeshProUGUI tm = GetComponent<TextMeshProUGUI>();
          if (tm != null)
          {
               if(tm.color != color)
               {
                    tm.color = color;
               }
          }
    }
}
