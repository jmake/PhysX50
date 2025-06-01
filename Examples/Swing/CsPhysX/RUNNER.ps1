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

    cp ..\Sources\*.* 
    cp ..\CMakeLists.txt . 

    cmake . "-DPHYSX_ROOT_DIR=${PHYSX_ROOT_DIR}"; cmake --build . --target ALL_BUILD --config Release;
    Set-Location -Path ${EXECUTION_PATH} 

    if (Test-Path Assets){Remove-Item -Recurse -Force Assets}
    mv "$FolderName\Assets" 

    if (Test-Path runme.exe){Remove-Item -Recurse -Force runme.exe}
    csc.exe /unsafe `
    ${EXECUTION_PATH}\Sources\runme.cs `
    ${EXECUTION_PATH}\Assets\Plugins\SpicyTech\ModuleName.cs `
    ${EXECUTION_PATH}\Assets\Plugins\SpicyTech\ModuleNamePINVOKE.cs `
    /platform:x64 /target:exe `
    /out:runme.exe `

    .\runme.exe
    if (Test-Path runme.exe){Remove-Item -Recurse -Force runme.exe}

    Set-Location -Path ${EXECUTION_PATH} 
    Write-Host "[COMPILATION] OK!"
}


<#---------------------------------------------------------------------------------------------#>
$EXECUTION_PATH=(Get-Location).Path 
Write-Host "[EXECUTION_PATH]:'${EXECUTION_PATH}'" 

try {csc.exe -version} catch{CL_SETUP}

try {
    nvcc.exe --version 
    if ($LASTEXITCODE -ne 0) {throw "'nvcc.exe --version' failed with exit code $LASTEXITCODE"} 
} 
catch{$env:PATH="$env:PATH;F:\z2025_1\Nvidia\bin;"}

try {
    swig.exe -version 
    if ($LASTEXITCODE -ne 0) {throw "'swig.exe -version' failed with exit code $LASTEXITCODE"} 
} 
catch{$env:PATH="$env:PATH;F:\z2025_1\PhysX\Swig\Swig431;"}

$PHYSX_ROOT_DIR="F:\z2025_1\PhysX\PhysX50\physx\PhysX50"
COMPILATION -FolderName "Exec"


Set-Location -Path ${EXECUTION_PATH} 
Write-Host "'" ($MyInvocation.MyCommand.Name) "' ..." 

<#---------------------------------------------------------------------------------------------#>