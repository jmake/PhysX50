Clear-Host
#try {deactivate} catch {Get-Location}

<#---------------------------------------------------------------------------------------------#>
function CL_SETUP
{
  echo "CL_SETUP ..."
   
  $VSWHERE="C:\ProgramData\Chocolatey\bin\vswhere.exe"

  $VSTOOLS = &($VSWHERE) -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
  Write-Output "[VSTOOLS]:'$VSTOOLS' "

  if($VSTOOLS) 
  {
    $VSTOOLS = join-path $VSTOOLS 'Common7\Tools\vsdevcmd.bat'
    if (test-path $VSTOOLS) 
    {
      cmd /s /c " ""$VSTOOLS""  -arch=x64 -host_arch=x64 $args && set" | where { $_ -match '(\w+)=(.*)' } | 
      foreach{$null = new-item -force -path "Env:\$($Matches[1])" -value $Matches[2] }
    }
  }
  
  cl.exe 
  cmake.exe --version 
  ninja.exe --version 
}


<#---------------------------------------------------------------------------------------------#>
function Vc16Win64CPU() 
{
    cp ..\Actions\vc16win64-cpu-only.xml buildtools\presets\public\vc16win64-cpu-only.xml
    .\generate_projects.bat vc16win64-cpu-only
    Set-Location -Path compiler\vc16win64-cpu-only 
}

function Vc16Win64GPU() 
{
    #if (Test-Path "physx\vc16win64") {Remove-Item -Recurse -Force "physx\vc16win64"}

    ## Compiler requires the CUDA toolkit.  Please set the CUDAToolkit_ROOT
    #$env:PATH="$env:PATH;F:\z2025_1\Nvidia\bin;" 

    cp ..\Actions\vc16win64-gpu.xml buildtools\presets\public\vc16win64-cpu-only.xml

    .\generate_projects.bat vc16win64-cpu-only

    Set-Location -Path compiler\vc16win64 
}


function COMPILATION
{
    Write-Host "[COMPILATION] ... " #-NoNewline

    #Get-Location 
    #Get-ChildItem 

    <# Generating 'physx\compiler\vc16win64-cpu-only\PhysXSDK.sln' 
    
    Error : "  Generator "Visual Studio 16 2019" could not find any instance of Visual Studio."

    1a) Modify 'physx\buildtools\presets\public\vc16win64-cpu-only.xml'
        compiler="vc16" -> compiler="vc17"

    1b) In 'physx\buildtools\cmake_generate_projects.py' 
        'vs_versions = {
            'vc15': '\"Visual Studio 15 2017\"',
            'vc16': '\"Visual Studio 16 2019\"',
            'vc17': '\"Visual Studio 17 2022\"'
        }'
    #>

    <# Generating 'physx\compiler\vc16win64-cpu-only\build.ninja' 

    1) Modify 'physx\buildtools\presets\public\vc16win64-cpu-only.xml' 
       add 'generator="ninja"'
       then  
            <platform targetPlatform="win64" compiler="vc16" generator="ninja" />

    #>
    if (Test-Path "bin") {Remove-Item -Recurse -Force "physx\bin"}

    Set-Location -Path physx 
    #Vc16Win64CPU
    #Vc16Win64GPU

    ninja.exe -f build-release.ninja -j4 install sdk_gpu_source_bin\all

    Set-Location -Path ${EXECUTION_PATH} 

    if (Test-Path "PhysX50.zip") {Remove-Item -Recurse -Force "PhysX50.zip"}
    Compress-Archive -Path "physx\PhysX50" -DestinationPath "PhysX50.zip"
    ls PhysX50.zip 

    #ls .\physx\bin\win.x86_64.vc143.mt\release\SnippetDeformableMesh_64.exe

    Write-Host "[COMPILATION] OK!"
}


<#---------------------------------------------------------------------------------------------#>
$EXECUTION_PATH=(Get-Location).Path 
Write-Host "[EXECUTION_PATH]:'${EXECUTION_PATH}'" 

try {cl.exe} catch{CL_SETUP}
COMPILATION 

Set-Location -Path ${EXECUTION_PATH} 
Write-Host "'" ($MyInvocation.MyCommand.Name) "' ..." 

<#

cl.exe               # Microsoft (R) C/C++ Optimizing Compiler Version 19.39.33523 for x64
cmake.exe --version  # cmake version 3.28.0-msvc1
ninja.exe --version  # 1.11.0 

#>
