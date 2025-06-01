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

    SetDllDirectory("Assets\\Plugins");
    ModuleName.myArrayCopy( source, target, target.Length );
          
    Console.WriteLine( "Contents of copy target array using default marshaling" );
    PrintArray( target );

    target = new int[ source.Length ];
    
    ModuleName.myArrayCopyUsingFixedArrays( source, target, target.Length );
    Console.WriteLine( "Contents of copy target array using fixed arrays" );
    PrintArray( target );

    target = new int[] { 4, 5, 6 };
    ModuleName.myArraySwap( source, target, target.Length );
    Console.WriteLine( "Contents of arrays after swapping using default marshaling" );
    PrintArray( source );
    PrintArray( target );
    
    source = new int[] { 1, 2, 3 };
    target = new int[] { 4, 5, 6 };
    
    ModuleName.myArraySwapUsingFixedArrays( source, target, target.Length );
    Console.WriteLine( "Contents of arrays after swapping using fixed arrays" );
    ModuleName.myArrayPrint(source, source.Length);
    ModuleName.myArrayPrint(target, target.Length);

    ModuleName.CudaVersion();
    ModuleName.PhysxVersion();

  }
  

  static void PrintArray( int[] a ) 
  {
    foreach ( int i in a ) 
      Console.Write( "{0} ", i );
    Console.WriteLine();
  }
}

