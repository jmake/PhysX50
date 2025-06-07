using System;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

using System.IO;
using System.Collections.Generic;

using UnityEngine;


public class SpicyXPlugging : MonoBehaviour
{
    SpicyX sx; 
    public int istep = 0; 
    public int nSteps = 100; 

    VertexVisualizer[] visualizers; 

    List<String> files = new List<String>(); 
    List<MeshCreator> meshCreators = new List<MeshCreator>();

    void Awake()
    {
        /*
        files.Add("teddy"); 
        files.Add("buda2"); 
        files.Add("teapot"); 
        //files.Add("simplest"); 

        foreach (var fname in files) 
        {
            GameObject o = new GameObject( "Mesh_" + fname.ToUpper() ); 
            MeshCreator mc = o.AddComponent<MeshCreator>();

            mc.Init();  
            mc.Load($"{fname}_faces.dat", $"{fname}_verts.dat");
            mc.UpdateMesh(); 
            meshCreators.Add(mc); 
        }
        */
    }


    void OnDisable()
    {
        VisualizersClear(); 
    }


    void Start()
    {
        pause = false; 
        VisualizersCreate(); 
    }


    void FixedUpdate()
    {
        // Time.fixedDeltaTime
        VisualizersPause(); 
        VisualizersEvolve(-1); 

        //MeshCreateCube(); 
    }


    void VisualizersPause() 
    {
        /*
            Invalid vertex or triangle data.
            UnityEngine.Debug:LogError (object)
            VertexVisualizer:Start () (at Assets/Scenes/VertexVisualizer.cs:15)
        */
        if (Input.anyKeyDown) 
        {
            foreach (char c in Input.inputString) 
            {
                Debug.Log("[VisualizersEvolve] key:" + c);
                if(sx != null) sx.KeyPress((int)c); 
            }
        }

        if(pause)
        {
            if(sx != null) sx.KeyPress('p'); 
        }

        message = $"i: {istep}";
    }


    void VisualizersCreate()
    {
        if(sx != null) return;
        
        //VisualizerTest(); 
        sx = new SpicyX();
        MeshCreateCube(sx, baseObject);     

        int nDeformables = sx.Init();
        Debug.Log("[Tester] nDeformables: " + nDeformables);

        visualizers = new VertexVisualizer[nDeformables];
        for (int i = 0; i < visualizers.Length; i++)
        {
            //GameObject go = new GameObject("Visualizer_" + i);
            visualizers[i] = new GameObject("Visualizer_" + i).AddComponent<VertexVisualizer>();

            if( baseObject )
            {
                Debug.Log( visualizers[i].GetComponent<MeshRenderer>().material ); 
                visualizers[i].GetComponent<MeshRenderer>().material = baseObject.GetComponent<MeshRenderer>().material;
            }

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

//        runner = false; 
    }


    void VisualizersEvolve(int i) 
    {
        if(sx == null) 
        {
            Debug.Log("[VisualizersEvolve] 'sx == null'");
            return; 
        }

        istep = sx.Step(i); 

        int nBodies = sx.nBodies(); 
        for(int iBody=0; iBody< nBodies; iBody++)
        {
            int nbVertices = sx.PositionsInvMassSize(iBody);
            int nTriangles = sx.TrianglesSize(iBody); 

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
            int[] slice2 = triangles.Skip(3).Take(3).ToArray();
        } // iBody 

    } // Evolve 


    /*
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
    */


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


    void MeshCreateCube(SpicyX spicyX, GameObject obj)
    {
        if(obj == null)
        {
            Debug.Log("[MeshCreate] 'baseObject' is null!");
            return; 
        }


        Mesh mesh = obj.GetComponent<MeshFilter>().mesh;
        if(mesh == null)
        {
            Debug.Log("[MeshCreate] 'MeshFilter' is null!");
            return; 
        }

        int[] triangles; // = new int[]{}; 
        float[] vertices; // = new float[]{}; 

        int nVertices = mesh.vertices.Length; 
        int nTriangles = mesh.triangles.Length / 3;  

        Debug.Log("[MeshCreate] Vertex count: " + nVertices);
        Debug.Log("[MeshCreate] Triangle count: " + nTriangles);

        Matrix4x4 localToWorld = obj.transform.localToWorldMatrix;

        Vector3[] localVertices = mesh.vertices;
        Vector3[] worldVertices = new Vector3[localVertices.Length];

        for (int i = 0; i < localVertices.Length; i++)
        {
            worldVertices[i] = localToWorld.MultiplyPoint3x4(localVertices[i]);
        }
        ComputeCentroid(worldVertices); 

        triangles = mesh.triangles; 
        //vertices = vertices2Array(localVertices); 
        vertices = vertices2Array(worldVertices); 

        if(spicyX == null) return; 

        spicyX.MeshAdd(vertices, vertices.Length, triangles, triangles.Length); 
    }


    Vector3 ComputeCentroid(Vector3[] vertices)
    {
        Vector3 sum = Vector3.zero;
        foreach (var v in vertices) sum += v;
        Vector3 centroid = sum / vertices.Length;
        Debug.Log("Centroid: " + centroid);
        return centroid;
    }

    float[] vertices2Array(Vector3[] meshVertices)
    {
        float[] array = new float[meshVertices.Length * 3];

        for (int i = 0; i < meshVertices.Length; i++) 
        {
            array[i * 3 + 0] = meshVertices[i].x;
            array[i * 3 + 1] = meshVertices[i].y;
            array[i * 3 + 2] = meshVertices[i].z;
        }

        return array;
    }


    void OnGUI()
    {
        // Style for the label
        GUIStyle labelStyle = new GUIStyle();
        labelStyle.fontSize = 64;
        labelStyle.normal.textColor = Color.red;

        // Draw the label
        Rect labelRect = new Rect(10, 10, 500, 80);
        GUI.Label(labelRect, message, labelStyle);

        // Style for the button
        GUIStyle buttonStyle = new GUIStyle(GUI.skin.button);
        buttonStyle.fontSize = 64;
        buttonStyle.alignment = TextAnchor.MiddleCenter;

        // Draw a button below the label
        Rect buttonRect = new Rect(10, 100, 512, 128);
        if (GUI.Button(buttonRect, (pause)?("Pause"):("Play"), buttonStyle))
        {
            Debug.Log("Button clicked!");
            pause = !pause; 
        }
    }


    [HideInInspector] public string something = "'spicyX.MeshAdd' works correctly, however, there is a strange segmentation fault the first time you try to run it. Upon restarting Unity, everything works relatively smoothly";

    public GameObject baseObject; 

    string message;
    bool pause; 
}