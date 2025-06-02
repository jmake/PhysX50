using System;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

using UnityEngine;


public class SwigTest : MonoBehaviour
{
    void Awake()
    {
    }

    void Start()
    {
        int[] source = { -1, -2, -3 };
        int[] target = new int[ source.Length ];
        ArrayPrint(target); 

        int cuda = ModuleName.CudaVersion();
        Debug.Log("Cuda Version: " + cuda);

        float physx = ModuleName.PhysxVersion();
        Debug.Log("PhysX Version: " + physx);

        TestSpicyX();

        Debug.Log("[Start] ok!");
    }


    void TestSpicyX() 
    {
        SpicyX obj = new SpicyX();

        int nDeformables = obj.Init();
        Debug.Log("[Tester] nDeformables: " + nDeformables);

        for(int i=0; i<69; i++) 
        {
            obj.Step(i);
            
            int nBodies = obj.nBodies(); 
            Debug.Log("[Tester] nBodies: " + nBodies);

            for(int iBody=0; iBody< nBodies; iBody++)
            {
                int nbVertices = obj.PositionsInvMassSize(iBody);
                Debug.Log("[Tester] nbVertices: " + nbVertices / 4);

                int nTriangles = obj.TrianglesSize(iBody); 
                Debug.Log("[Tester] nTriangles: " + nTriangles / 3);

                float[] vertices = new float[nbVertices];
                obj.PositionsInvMassGet(iBody, vertices); 

                int[] triangles = new int[nTriangles];
                obj.TrianglesGet(iBody, triangles); 

                float[] slice1 = vertices.Skip(2).Take(3).ToArray();
                Debug.Log(string.Join(", ", slice1)); 

                int[] slice2 = triangles.Skip(3).Take(3).ToArray();
                Debug.Log(string.Join(", ", slice2)); 
            } // iBody 

        } // itime  

        obj.Finish();
    } // Main 


    void ArrayPrint(int[] array)
    {
        string result = "[" + string.Join(", ", array) + "]";
        Debug.Log(result);
    }

}