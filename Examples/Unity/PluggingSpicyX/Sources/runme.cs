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

    int cuda = ModuleName.CudaVersion();
    Console.WriteLine("[Tester] CUDA Version: " + cuda);

    float physx = ModuleName.PhysxVersion();
    Console.WriteLine("[Tester] PhysX Version: " + physx);

    int spicyx = ModuleName.SpicyTest();
    Console.WriteLine("[Tester] SpicyX: " + spicyx);

    if (spicyx != 69) throw new System.Exception("[ASSERT FAILED] SpicyTest fails!!");

SpicyX obj = new SpicyX(10);
Console.WriteLine("[Tester] Initial value: " + obj.GetValue());
obj.SetValue(42);
Console.WriteLine("[Tester] New value: " + obj.GetValue());

obj.InitFlatArray(10); 
int size = obj.GetFlatArraySize();
float[] result = new float[size];
obj.GetFlatArrayRaw(result);

Console.WriteLine("[Tester] size: " + size);
Console.WriteLine("[Tester] vector: [" + string.Join(", ", result) +"] ");


  }
  
}