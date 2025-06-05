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

    SpicyXTestB(3); 
    SpicyXTestA(69); 
    SpicyXTestB(7); 

    Console.WriteLine("[Tester] Finish");
  } // Main 


  static void SpicyXTestB(int nTime=0) 
  {
    SpicyX obj = new SpicyX();
    MeshCreateCube(obj);     
    int nDeformables = obj.Init();

    Console.WriteLine("[Tester] nDeformables: " + nDeformables);

    obj.Finish();
  } 


  static void SpicyXTestA(int nTime) 
  {
    SpicyX obj = new SpicyX();

    int nDeformables = obj.Init();
    Console.WriteLine("[Tester] nDeformables: " + nDeformables);

    obj.KeyPress('P'); 
    obj.KeyPress('P'); 
    for(int i=0; i<nTime; i++) VisualizersEvolve(obj, i); 

    obj.KeyPress('R'); 
    obj.KeyPress('o'); 

    obj.Finish();
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


  static void MeshCreateCube(SpicyX obj) 
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
    obj.MeshAdd(vertices, vertices.Length, triangles, triangles.Length);     
  }

} // runme