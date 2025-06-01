
## See 

https://github.com/swig/swig

https://medium.com/@jmake/an-python-interface-for-physx-bd221c9b06a

https://www.swig.org/Doc4.3/Contents.html#Contents 

https://www.swig.org/Doc4.3/CSharp.html#CSharp


## Test1b1
```
>> .\RUNNER.ps1 
[EXECUTION_PATH]:'F:\z2025_1\PhysX\Swig\Test1a1'
Microsoft (R) C/C++ Optimizing Compiler Version 19.39.33523 for x64
Copyright (C) Microsoft Corporation.  All rights reserved.

usage: cl [ option... ] filename... [ /link linkoption... ]
nvcc: NVIDIA (R) Cuda compiler driver
Copyright (c) 2005-2025 NVIDIA Corporation
Built on Wed_Jan_15_19:38:46_Pacific_Standard_Time_2025
Cuda compilation tools, release 12.8, V12.8.61
Build cuda_12.8.r12.8/compiler.35404655_0

SWIG Version 4.3.1

Compiled with x86_64-w64-mingw32-g++ [x86_64-w64-mingw32]

Configured options: +pcre

Please see https://www.swig.org for reporting bugs and further information
[COMPILATION] ... 


    Directory: F:\z2025_1\PhysX\Swig\Test1a1


Mode                 LastWriteTime         Length Name
----                 -------------         ------ ----
d-----          05/31/25     22:51                Exec
-- Building for: Visual Studio 17 2022
-- Selecting Windows SDK version 10.0.22621.0 to target Windows 10.0.26120.
-- The C compiler identification is MSVC 19.39.33523.0
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.39.33519/bin/Hostx64/x64/cl.exe - skipped
-- Detecting C compile features
-- Detecting C compile features - done
[WRAPPER] PHYSX_ROOT_DIR='F:\z2025_1\PhysX\PhysX50\physx\PhysX50'
-- Found SWIG: F:/z2025_1/PhysX/Swig/Swig431/swig.exe (found version "4.3.1")  
-- Configuring done (1.7s)
-- Generating done (0.0s)
-- Build files have been written to: F:/z2025_1/PhysX/Swig/Test1a1/Exec
MSBuild version 17.9.8+b34f75857 for .NET Framework

  1>Checking Build System
  Swig compile example.i for csharp
  Building Custom Rule F:/z2025_1/PhysX/Swig/Test1a1/Exec/CMakeLists.txt
  exampleCSHARP_wrap.c
  example.c
  Generating Code...
     Creating library F:/z2025_1/PhysX/Swig/Test1a1/Exec/Release/LibraryName.lib and object F:/z2025_1/Ph
  ysX/Swig/Test1a1/Exec/Release/LibraryName.exp
  LibraryName.vcxproj -> F:\z2025_1\PhysX\Swig\Test1a1\Exec\Release\LibraryName.dll
  Building Custom Rule F:/z2025_1/PhysX/Swig/Test1a1/Exec/CMakeLists.txt


    Directory: F:\z2025_1\PhysX\Swig\Test1a1\Exec\Release


Mode                 LastWriteTime         Length Name
----                 -------------         ------ ----
-a----          05/31/25     22:51           1526 example.cs
-a----          05/31/25     22:51          13806 exampleCSHARP_wrap.c
-a----          05/31/25     22:51          12889 examplePINVOKE.cs
-a----          05/31/25     22:51          10240 LibraryName.dll
-a----          05/31/25     22:51           1830 LibraryName.exp
-a----          05/31/25     22:51           3600 LibraryName.lib
Microsoft (R) Visual C# Compiler version 4.9.0-3.24121.1 (a98c90d5)
Copyright (C) Microsoft Corporation. All rights reserved.

Contents of copy target array using default marshaling
1 2 3 
Contents of copy target array using fixed arrays
1 2 3
Contents of arrays after swapping using default marshaling
4 5 6
1 2 3
Contents of arrays after swapping using fixed arrays
4 5 6
1 2 3
[COMPILATION] OK!
' RUNNER.ps1 ' ...


PS F:\z2025_1\PhysX\Swig\Test1a1> 
```
