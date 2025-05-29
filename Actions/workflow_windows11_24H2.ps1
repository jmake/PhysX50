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

    cp .\Actions\vc16win64-cpu-only.xml physx\buildtools\presets\public\vc16win64-cpu-only.xml

    ## Compiler requires the CUDA toolkit.  Please set the CUDAToolkit_ROOT
    ##cp .\Actions\vc16win64-gpu.xml physx\buildtools\presets\public\vc16win64-cpu-only.xml

    Set-Location -Path physx 
    .\generate_projects.bat vc16win64-cpu-only

    Set-Location -Path compiler\vc16win64-cpu-only 
    ninja.exe -j4 install 

    Set-Location -Path ${EXECUTION_PATH} 

    ##tar -cvf PhysX50.tar physx\PhysX50
    ##ls PhysX50.tar

    Compress-Archive -Path "physx\PhysX50" -DestinationPath "PhysX50.zip"
    ls PhysX50.zip 
    
    ls .\physx\bin\win.x86_64.vc143.mt\release\SnippetDeformableMesh_64.exe

    Write-Host "[COMPILATION] OK!"
}


<#---------------------------------------------------------------------------------------------#>
$EXECUTION_PATH=(Get-Location).Path 
Write-Host "[EXECUTION_PATH]:'${EXECUTION_PATH}'" 

CL_SETUP
COMPILATION 
