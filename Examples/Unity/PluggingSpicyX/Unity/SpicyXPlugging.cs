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

    bool runner = false;  

    VertexVisualizer[] visualizers; 


    void OnDisable()
    {
        VisualizersClear(); 
    }


    void Start()
    {
        VisualizersCreate(); 
    }


    void FixedUpdate()
    {
        // Time.fixedDeltaTime
        VisualizersEvolve(++istep); 
    }

    void VisualizersCreate()
    {
        if(sx != null) return;
        
        //VisualizerTest(); 
        sx = new SpicyX();

        int nDeformables = sx.Init();
        Debug.Log("[Tester] nDeformables: " + nDeformables);

        visualizers = new VertexVisualizer[nDeformables];
        for (int i = 0; i < visualizers.Length; i++)
        {
            //GameObject go = new GameObject("Visualizer_" + i);
            visualizers[i] = new GameObject("Visualizer_" + i).AddComponent<VertexVisualizer>();
        }
    }


    void VisualizersClear()
    {
        if (visualizers != null)
        {
            for (int i = 0; i < visualizers.Length; i++)
            {
                if (visualizers[i] != null)
                {
                    Destroy(visualizers[i].gameObject);
                }
            }

            visualizers = null; // release the reference
        }

        if(sx != null) 
        {
            sx.Finish(); // Crash??
            sx = null; 
        }
        istep = 0; 

        runner = false; 
    }


    void VisualizersEvolve(int i) 
    {
        if(sx == null) 
        {
            Debug.Log("[VisualizersEvolve] 'sx == null'");
            return; 
        }

        int itime = sx.Step(i);
        //Debug.Log("[Tester] itime: " + itime + " i: " + i);
        
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

            if (visualizers[iBody].vertices == null || visualizers[iBody].vertices.Length != vertices.Length)
                visualizers[iBody].vertices = new float[nbVertices];
            Array.Copy(vertices, visualizers[iBody].vertices, vertices.Length);


            int[] triangles = new int[nTriangles];
            sx.TrianglesGet(iBody, triangles); 
            if (visualizers[iBody].triangles == null || visualizers[iBody].triangles.Length != triangles.Length)
                visualizers[iBody].triangles = new int[nTriangles];
            Array.Copy(triangles, visualizers[iBody].triangles, triangles.Length);


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
            } // iBody 

        } // itime  

        obj.Finish();
    } // Main 


    void VisualizerTest()
    {
        GameObject go = new GameObject("VertexVisualizer");
        VertexVisualizer visualizer = go.AddComponent<VertexVisualizer>();

        int nb = 4;
        visualizer.vertices = new float[nb * 4];

        Vector3[] squareVerts = new Vector3[]
        {
            new Vector3(0, 0, 0),   // 0: bottom left
            new Vector3(1, 0, 0),   // 1: bottom right
            new Vector3(0, 1, 0),   // 2: top left
            new Vector3(1, 1, 0)    // 3: top right
        };

        for (int i = 0; i < nb; i++)
        {
            visualizer.vertices[i * 4 + 0] = squareVerts[i].x;
            visualizer.vertices[i * 4 + 1] = squareVerts[i].y;
            visualizer.vertices[i * 4 + 2] = squareVerts[i].z;
            visualizer.vertices[i * 4 + 3] = 1f; // mass
        }

        // Define triangle indices (2 triangles: [0,1,3], [0,3,2])
        visualizer.triangles = new int[]{0, 1, 3, 0, 3, 2};
    }


}