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
    example.myArrayCopy( source, target, target.Length );
          
    Console.WriteLine( "Contents of copy target array using default marshaling" );
    PrintArray( target );

    target = new int[ source.Length ];
    
    example.myArrayCopyUsingFixedArrays( source, target, target.Length );
    Console.WriteLine( "Contents of copy target array using fixed arrays" );
    PrintArray( target );

    target = new int[] { 4, 5, 6 };
    example.myArraySwap( source, target, target.Length );
    Console.WriteLine( "Contents of arrays after swapping using default marshaling" );
    PrintArray( source );
    PrintArray( target );
    
    source = new int[] { 1, 2, 3 };
    target = new int[] { 4, 5, 6 };
    
    example.myArraySwapUsingFixedArrays( source, target, target.Length );
    Console.WriteLine( "Contents of arrays after swapping using fixed arrays" );
    example.myArrayPrint(source, source.Length);
    example.myArrayPrint(target, target.Length);
  }
  

  static void PrintArray( int[] a ) 
  {
    foreach ( int i in a ) 
      Console.Write( "{0} ", i );
    Console.WriteLine();
  }
}

