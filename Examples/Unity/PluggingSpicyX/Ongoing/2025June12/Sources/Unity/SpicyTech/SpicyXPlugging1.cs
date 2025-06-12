using System;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

using System.IO;
using System.Collections.Generic;

using UnityEngine;


public class SpicyXPlugging1 : MonoBehaviour
{
    [Header("set 'PxRigidBodyFlag::eKINEMATIC == true' ")]
    public bool kinematic = true; 

    public int istep = 0; 

    VertexVisualizer[] visualizersDeformables; 
    List<SpicyRigidBody> rigidBodies = new List<SpicyRigidBody>(); 

    SpicyX sx; 

    void Awake()
    {
    }


    void OnDisable()
    {
        VisualizersClear(); 
    }


    void Start()
    {
        pause = false; 
        VisualizersStart(); 
    }


    void FixedUpdate()
    {
        // Time.fixedDeltaTime
        VisualizersPause(); 
        VisualizersEvolve(-1); 

        RigidBodiesEvolve(sx); 
    }


    void VisualizersPause() 
    {
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


    void VisualizersStart()
    {
        if(sx != null) return;

        bool showDate = true; 
        bool appendLog = false; 
        sx = new SpicyX(appendLog, showDate);

        DeformablesInit(sx); 
        RigidBodiesInit(sx); 

        sx.Init();
        int nRigids = sx.HardSize(); 
        int nDeformables = sx.SoftSize(); 
        Debug.Log("[Tester] nRigids: " + nRigids);
        Debug.Log("[Tester] nDeformables: " + nDeformables);

        VisualizersDeformablesCreate( nDeformables ); 
    }


    void RigidBodiesEvolve(SpicyX spicyX) 
    {
        foreach (SpicyRigidBody rb in rigidBodies)
        {
            //rb.GlobalPoseGet(spicyX); 
        }

        if(kinematic)
        {

        }
        else
        {
            foreach (SpicyRigidBody rb in rigidBodies)
            {
                rb.GlobalPoseSet(spicyX); 
            }
        }
    } 


    void RigidBodiesInit(SpicyX spicyX) 
    {
        foreach (GameObject hardObject in hardObjects)
        {
            GameObject go = new GameObject("Rigid_" + hardObject.name); 
            SpicyRigidBody rb = go.AddComponent<SpicyRigidBody>(); 
            rb.RigidBodySet(hardObject, rigidBodies.Count); 

            Vector3 p0 = rb.PositionGet(); 
            Debug.Log("p0:" + p0 ); 

            Quaternion q0 = rb.QuaternionGet(); 
            Debug.Log("q0:" + q0.eulerAngles + " ["+ q0 +"]"); 

            MeshCreateHard(spicyX, hardObject, p0, q0);

            rigidBodies.Add( rb ); 
        }
    } 


    void VisualizersDeformablesCreate(int nDeformables)
    {
        visualizersDeformables = new VertexVisualizer[nDeformables];
        for (int i = 0; i < visualizersDeformables.Length; i++)
        {
            GameObject go = new GameObject("Soft_" + i); 
            visualizersDeformables[i] = go.AddComponent<VertexVisualizer>(); 

            foreach (GameObject softObject in softObjects)
            {
                visualizersDeformables[i].GetComponent<MeshRenderer>().material = softObject.GetComponent<MeshRenderer>().material;
            }
        }
    }


    void DeformablesInit(SpicyX spicyX) 
    {
        foreach (GameObject softObject in softObjects)
        {
            Debug.Log(softObject.name);
            MeshCreateSoft(spicyX, softObject);
        }
    }


    void VisualizersClear()
    {
        if (visualizersDeformables != null)
        {
            for (int i = 0; i < visualizersDeformables.Length; i++)
            {
                if (visualizersDeformables[i] != null)
                {
                    Destroy(visualizersDeformables[i].gameObject);
                }
            }

            visualizersDeformables = null; // release the reference
        }

        if(sx != null) 
        {
            sx.Finish(); // Crash??
            sx = null; 
        }
        istep = 0; 
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

            if (visualizersDeformables[iBody].vertices == null || visualizersDeformables[iBody].vertices.Length != vertices.Length)
                visualizersDeformables[iBody].vertices = new float[nbVertices];
            Array.Copy(vertices, visualizersDeformables[iBody].vertices, vertices.Length);


            int[] triangles = new int[nTriangles];
            sx.TrianglesGet(iBody, triangles); 
            if (visualizersDeformables[iBody].triangles == null || visualizersDeformables[iBody].triangles.Length != triangles.Length)
                visualizersDeformables[iBody].triangles = new int[nTriangles];
            Array.Copy(triangles, visualizersDeformables[iBody].triangles, triangles.Length);

            float[] slice1 = vertices.Skip(2).Take(3).ToArray();
            int[] slice2 = triangles.Skip(3).Take(3).ToArray();
        } // iBody 

    } // Evolve 


    void MeshCreateHard(SpicyX spicyX, GameObject obj, Vector3 p, Quaternion q)
    {
        spicyX.GlobalPoseInitial(p[0],p[1],p[2], q[0],q[1],q[2],q[3]); 
        MeshCreate(spicyX, obj, new MeshAddDelegate(spicyX.HardAdd), false, kinematic);
    }


    void MeshCreateSoft(SpicyX spicyX, GameObject obj)
    {
        MeshCreate(spicyX, obj, new MeshAddDelegate(spicyX.SoftAdd), true);
    }


    void MeshCreate(
        SpicyX spicyX, 
        GameObject obj, 
        MeshAddDelegate function, 
        bool global, 
        bool kinematic=false
    )
    {
        if(obj == null)
        {
            Debug.Log("[MeshCreate] 'softObject' is null!");
            return; 
        }


        Mesh mesh = obj.GetComponent<MeshFilter>().mesh;
        if(mesh == null)
        {
            Debug.Log("[MeshCreate] 'MeshFilter' is null!");
            return; 
        }


        int[] triangles = mesh.triangles; 
        int nTriangles = triangles.Length / 3;  

        Vector3[] localVertices = mesh.vertices;
        int nVertices = localVertices.Length; 

        Debug.Log("[MeshCreate] Vertex count: " + nVertices);
        Debug.Log("[MeshCreate] Triangle count: " + nTriangles);

        float[] vertices; 

        if(global)
        {
            Matrix4x4 localToWorld = obj.transform.localToWorldMatrix;
            Vector3[] worldVertices = new Vector3[localVertices.Length];
            for (int i = 0; i < localVertices.Length; i++) worldVertices[i] = localToWorld.MultiplyPoint3x4(localVertices[i]);
            //ComputeCentroid(worldVertices); 
            vertices = vertices2Array(worldVertices); 
        }
        else
        {
            vertices = vertices2Array(localVertices); 
        }

        if(spicyX == null) return; 

        function(vertices, vertices.Length, triangles, triangles.Length, kinematic); 
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

    public GameObject[] softObjects; 
    public GameObject[] hardObjects; 

    delegate void MeshAddDelegate(
        float[] vertices, 
        int verticesLength, 
        int[] triangles, 
        int trianglesLength, 
        bool kinematic
    );

    string message;
    bool pause; 
}

/*
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
*/
