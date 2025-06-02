using System;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

using UnityEngine;


public class SpicyXPlugging : MonoBehaviour
{
    SpicyX sx; 
    public int nSteps = 100; 
    public int istep = 0; 

    void Start()
    {
        //if(sx) delete sx; 
        sx = new SpicyX();
        //sx.Finish(); // Crash!!

        int nDeformables = sx.Init();
        Debug.Log("[Tester] nDeformables: " + nDeformables);

        //for(int i=0; i < nSteps; i++) Evolve(i); sx.Finish();
    }


    void OnDisable()
    {
        sx.Finish(); // Crash??
        istep = 0; 
    }


    void FixedUpdate()
    {
        // Time.fixedDeltaTime
        Evolve(++istep); // Crash??
    }


    void Evolve(int i) 
    {
        int itime = sx.Step(i);
        Debug.Log("[Tester] itime: " + itime + " i: " + i);
        
        int nBodies = sx.nBodies(); 
        //Debug.Log("[Tester] nBodies: " + nBodies);

        for(int iBody=0; iBody< nBodies; iBody++)
        {
            int nbVertices = sx.PositionsInvMassSize(iBody);
            //Debug.Log("[Tester] nbVertices: " + nbVertices / 4);

            int nTriangles = sx.TrianglesSize(iBody); 
            //Debug.Log("[Tester] nTriangles: " + nTriangles / 3);

            float[] vertices = new float[nbVertices];
            sx.PositionsInvMassGet(iBody, vertices); 

            int[] triangles = new int[nTriangles];
            sx.TrianglesGet(iBody, triangles); 

            float[] slice1 = vertices.Skip(2).Take(3).ToArray();
            //Debug.Log(string.Join(", ", slice1)); 

            int[] slice2 = triangles.Skip(3).Take(3).ToArray();
            //Debug.Log(string.Join(", ", slice2)); 
        } // iBody 

    } // Evolve 


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

}