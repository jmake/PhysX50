using System.Runtime.InteropServices;
using UnityEngine;
using System;
using System.Text;



public class SwigTest : MonoBehaviour
{
    //[DllImport("LibraryName")]
    //private static extern void myArrayPrint(int[] array4, int nitems);
/*
void Awake()
{
    try {
        int[] source = { 1, 2, 3 }; 
        myArrayPrint(source, source.Length); // or any other safe native function
    }
    catch (DllNotFoundException e) {
        Debug.LogError("DLL not found: " + e.Message);
    }
}
*/
    void Start()
    {
        int[] source = { -1, -2, -3 };
        int[] target = new int[ source.Length ];

        ArrayPrint(target); 
        example.myArrayCopy( source, target, target.Length );
        //example.myArrayPrint(source, source.Length);        
        ArrayPrint(target); 

        Debug.Log("[Start] ok!");
    }

    void ArrayPrint(int[] array)
    {
        string result = "[" + string.Join(", ", array) + "]";
        Debug.Log(result);
    }
}