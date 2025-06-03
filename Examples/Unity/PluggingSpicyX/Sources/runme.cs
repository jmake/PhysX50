using System;
using System.Linq;
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

    SpicyX obj = new SpicyX();

    int nDeformables = obj.Init();
    Console.WriteLine("[Tester] nDeformables: " + nDeformables);

    obj.KeyPress('P'); 
    obj.KeyPress('P'); 
    for(int i=0; i<69; i++) VisualizersEvolve(obj, i); 

    obj.KeyPress('R'); 
    obj.KeyPress('o'); 

    obj.Finish();

    Console.WriteLine("[Tester] Finish");
  } // Main 


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

  } // itime  



} // runme