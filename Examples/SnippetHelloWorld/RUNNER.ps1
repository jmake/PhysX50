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
    param([string]$FolderName) 

    Write-Host "[COMPILATION] ... " #-NoNewline

    if (Test-Path $FolderName){Remove-Item -Recurse -Force $FolderName}
    New-Item -ItemType Directory -Path $FolderName 
    Set-Location -Path $FolderName

    cmake .. `
    -G "Ninja" `
    "-DPHYSX_ROOT_DIR=${PHYSX_ROOT_DIR}"

    ninja.exe -j4  

    cp F:\z2025_1\PhysX\PhysX50\physx\PhysX50\bin\win.x86_64.vc143.mt\release\*.dll .
    .\snippetHelloWorld.exe 
#    more .\test_output.txt

#    ctest.exe 

    Set-Location -Path ${EXECUTION_PATH} 
    Write-Host "[COMPILATION] OK!"
}


<#---------------------------------------------------------------------------------------------#>
$EXECUTION_PATH=(Get-Location).Path 
Write-Host "[EXECUTION_PATH]:'${EXECUTION_PATH}'" 

CL_SETUP

$PHYSX_ROOT_DIR="F:\z2025_1\PhysX\PhysX50\physx\PhysX50"
COMPILATION -FolderName "Exec"


Set-Location -Path ${EXECUTION_PATH} 
Write-Host "'" ($MyInvocation.MyCommand.Name) "' ..." 

<#---------------------------------------------------------------------------------------------#>