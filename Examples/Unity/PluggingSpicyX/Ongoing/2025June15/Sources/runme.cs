using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using System.Runtime.InteropServices;

public class runme
{
  [DllImport("kernel32.dll", SetLastError = true)]
  static extern bool SetDllDirectory(string lpPathName);  

  static void Main() 
  {
    int[] source = { 1, 2, 3 };
    int[] target = new int[ source.Length ];

    //SetDllDirectory("Assets\\Plugins");
    SetDllDirectory(".");

    int cuda = ModuleName.CudaVersion();
    Console.WriteLine("[Tester] CUDA Version: " + cuda);

    float physx = ModuleName.PhysxVersion();
    Console.WriteLine("[Tester] PhysX Version: " + physx);

    LoadFileFlatTest(); 

    SpicyXTestC(3); 

    System.Threading.Thread.Sleep(100); 
    SpicyXTestA(69); 
    System.Threading.Thread.Sleep(100); 
    SpicyXTestB(7); 

    Console.WriteLine("[Tester] Finish");
  } // Main 


  static void SpicyXTestC(int nTime=0) 
  {
    using (SpicyX obj = new SpicyX(true))
    {
      float[] p0 = {0.001f, 0.002f, 0.003f};
      float[] q0 = {0.0f, 0.0f, 0.0f, 1.0f};

      MeshCreateHard(obj, p0, q0); 
      obj.Init();
      int nRigids = obj.HardSize(); 
      int nDeformables = obj.SoftSize(); 
      Console.WriteLine("[Tester] nDeformables: " + nDeformables);

      int ibody = 0; 
      float[] position = {0.0f, 0.0f, 0.0f};
      obj.PositionGet(ibody, position); 
      Console.WriteLine("[Tester] position: " + position[0] +" "+ position[1] +" "+ position[2]);

      int itime = 0; 
      //obj.PositionSet(ibody, 0.0f, 0.0f, 0.0f); 
      obj.GlobalPoseSet(ibody, itime, 1.0f,2.0f,3.0f, 4.0f,5.0f,6.0f,7.0f); 
      VisualizersEvolve(obj, itime); 

      //obj.PositionGet(ibody, position); 
      //Console.WriteLine("[Tester] position: " + position[0] +" "+ position[1] +" "+ position[2]);

      float[] p = {0.0f, 0.0f, 0.0f};
      float[] q = {0.0f, 0.0f, 0.0f, 0.0f};
      obj.GlobalPoseGet(ibody, p, q); 
      Console.WriteLine("[Tester] pose.p: " + p[0] +" "+ p[1] +" "+ p[2]);
      Console.WriteLine("[Tester] pose.q: " + q[0] +" "+ q[1] +" "+ q[2] +" "+ q[3]);

      obj.Finish();
    }    
  } 

  static void SpicyXTestB(int nTime=0) 
  {
    using (SpicyX obj = new SpicyX(true))
    {
      MeshCreateSoft(obj); 
      //int nDeformables = 
      obj.Init();
      int nRigids = obj.HardSize(); 
      int nDeformables = obj.SoftSize(); 
      Console.WriteLine("[Tester] nDeformables: " + nDeformables);
      obj.Finish();
    } 
  } 


  static void SpicyXTestA(int nTime) 
  {
    using (SpicyX obj = new SpicyX(true))
    {
        //int nDeformables = 
        obj.Init();
        int nRigids = obj.HardSize(); 
        int nDeformables = obj.SoftSize(); 
        Console.WriteLine("[Tester] nDeformables: " + nDeformables);

        obj.KeyPress('P'); 
        obj.KeyPress('P'); 
        for(int i=0; i<nTime; i++) VisualizersEvolve(obj, i); 

        obj.KeyPress('R'); 
        obj.KeyPress('o'); 

        obj.Finish();
    }
  }


  static void VisualizersEvolve(SpicyX obj, int i) 
  {
    int itime = obj.Step(i);
    Console.WriteLine("[Tester] itime: " + itime);
    
    int nBodies = obj.nBodies(); 
    //Console.WriteLine("[Tester] nBodies: " + nBodies);

    for(int iBody=0; iBody< nBodies; iBody++)
    {
      int nbVertices = obj.PositionsInvMassSize(iBody);
      //Console.WriteLine("[Tester] nbVertices: " + nbVertices / 4);

      int nTriangles = obj.TrianglesSize(iBody); 
      //Console.WriteLine("[Tester] nTriangles: " + nTriangles / 3);

      float[] vertices = new float[nbVertices];
      obj.PositionsInvMassGet(iBody, vertices); 

      int[] triangles = new int[nTriangles];
      obj.TrianglesGet(iBody, triangles); 

      float[] slice1 = vertices.Skip(2).Take(3).ToArray();
      //Console.WriteLine(string.Join(", ", slice1)); 

      int[] slice2 = triangles.Skip(3).Take(3).ToArray();
      //Console.WriteLine(string.Join(", ", slice2)); 
    } // iBody 

  } // VisualizersEvolve


  static void LoadFileFlatTest()
  {
    string filePath = "Scripts\\teapot_faces.dat"; 
    LoadFileFlat(filePath); 

  }


  static List<List<float>> 
  LoadFileFlat(string filePath)
  {
    filePath = Path.GetFullPath(filePath);
    Console.WriteLine("[LoadFileFlat] filePath:'" + filePath +"' "); 

    List<List<float>> matrix = new List<List<float>>();
    if( !File.Exists(filePath) ) return matrix; 

    int rows = 0, cols = 0;
    FloatVector flatData = ModuleName.LoadFileFlat(filePath, out rows, out cols);

    for (int i = 0; i < rows; i++) {
        List<float> row = new List<float>();
        for (int j = 0; j < cols; j++) {
            row.Add(flatData[i * cols + j]);
        }
        matrix.Add(row);
    }

    Console.WriteLine("[LoadFileFlat] matrix:'" + matrix.Count +"' "); 

    //PrintMatrix(matrix);
    return matrix;
  } // LoadFileFlatTest


  static void PrintMatrix(List<List<float>> matrix)
  {
      foreach (var row in matrix)
      {
          Console.WriteLine(string.Join(", ", row));
      }

      Console.WriteLine("matrix:" + matrix.Count ); 
  }


  static void MeshCreateSoft(SpicyX spicyX)
  {
    MeshCreate(spicyX, false, new MeshAddDelegate(spicyX.SoftAdd));
  }


  static void MeshCreateHard(SpicyX spicyX, float[] p, float[] q)
  {
    spicyX.GlobalPoseInitial(p[0],p[1],p[2], q[0],q[1],q[2],q[3]); 
    MeshCreate(spicyX, true, new MeshAddDelegate(spicyX.HardAdd));
  }


  static void MeshCreate(SpicyX obj, bool kinematic, MeshAddDelegate function) 
  {
    // Unity C# 'Vector3[]'
    List<List<float>> meshVertices = new List<List<float>> {
        new List<float> {  0.5f, -0.5f, -0.5f },
        new List<float> {  0.5f, -0.5f,  0.5f },
        new List<float> { -0.5f, -0.5f,  0.5f },
        new List<float> { -0.5f, -0.5f, -0.5f },
        new List<float> {  0.5f,  0.5f, -0.5f },
        new List<float> {  0.5f,  0.5f,  0.5f },
        new List<float> { -0.5f,  0.5f,  0.5f },
        new List<float> { -0.5f,  0.5f, -0.5f }
    };

    int[] triangles = new int[] {
      1, 2, 3,
      7, 6, 5,
      4, 5, 1,
      5, 6, 2,
      2, 6, 7,
      0, 3, 7,
      0, 1, 3,
      4, 7, 5,
      0, 4, 1,
      1, 5, 2,
      3, 2, 7,
      4, 0, 7
    };


    int nDims = 3; 
    int nVertices = meshVertices.Count;   

    int nIndices = 3; 
    int nTriangles = triangles.Length / nIndices; 

    float[] vertices = new float[nVertices * nDims];
    for (int i = 0; i < nVertices; i++)
    {
        for (int j = 0; j < nDims; j++)
        {
            vertices[nDims * i + j] = meshVertices[i][j];
        }
    }

    /*
    InitPhysics -> initScene -> createDeformableVolumes -> createDeformableVolume
    */
    //obj.SoftAdd(vertices, vertices.Length, triangles, triangles.Length);  
    function(vertices, vertices.Length, triangles, triangles.Length, kinematic);   
  }

  delegate void MeshAddDelegate(
    float[] vertices, 
    int verticesLength, 
    int[] triangles, 
    int trianglesLength, 
    bool kinematic
  );

} // runme