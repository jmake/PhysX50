using System;
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

    ModuleName.myArrayCopy( source, target, target.Length );    
    ModuleName.myArrayPrint(source, source.Length);
    ModuleName.myArrayPrint(target, target.Length);

    int cuda = ModuleName.CudaVersion();
    Console.WriteLine("[Tester] CUDA Version: " + cuda);

    float physx = ModuleName.PhysxVersion();
    Console.WriteLine("[Tester] PhysX Version: " + physx);

int spicyx = ModuleName.SpicyTest();
Console.WriteLine("[Tester] SpicyX: " + spicyx);
  }
  
}