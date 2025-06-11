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
function Verify-FileHash 
{
    param (
        [Parameter(Mandatory = $true)][string]$FilePath, 
        [Parameter(Mandatory = $true)][string]$ExpectedHash
    )

    $actualHash = (Get-FileHash -Path $FilePath -Algorithm SHA256).Hash
    if ($actualHash -ne $ExpectedHash) {
        Write-Host "[RUNNER] '${FilePath}' fails!!"
        Set-Location -Path ${EXECUTION_PATH}
        exit 1
    } else {
        Write-Host "[RUNNER] '${FilePath}' verified!!"
    }
}


<#---------------------------------------------------------------------------------------------#>
function COMPILATION
{
    param([string]$FolderName) 

    Write-Host "[COMPILATION] ... " #-NoNewline

    if (Test-Path $FolderName){Remove-Item -Recurse -Force $FolderName}
    New-Item -ItemType Directory -Path $FolderName 

    Set-Location -Path $FolderName
    #cp ${EXECUTION_PATH}\Sources\* . 
    Copy-Item -Path "${EXECUTION_PATH}\Sources\*" -Destination "." -Recurse

#Set-Location -Path ${EXECUTION_PATH};exit 

    cmake . "-DPHYSX_ROOT_DIR=${PHYSX_COMPILATION_DIR}"; 
    cmake --build . --target ALL_BUILD --config Release;
#Set-Location -Path ${EXECUTION_PATH} 

    if (Test-Path "${EXECUTION_PATH}\Assets"){Remove-Item -Recurse -Force "${EXECUTION_PATH}\Assets"}
    mv "Assets" ${EXECUTION_PATH}

    Set-Location -Path ${EXECUTION_PATH}
    Set-Location -Path "Assets\Plugins" 
    cp ${PHYSX_COMPILATION_DIR}\bin\win.x86_64.vc143.mt\release\*.dll .

    csc.exe /unsafe `
    ${EXECUTION_PATH}\Sources\runme.cs `
    ${EXECUTION_PATH}\Assets\Plugins\SpicyTech\*.cs `
    /platform:x64 /target:exe `
    /out:runme.exe `

#Set-Location -Path ${EXECUTION_PATH};exit 

    .\runme.exe
#    if (Test-Path runme.exe){Remove-Item -Recurse -Force runme.exe}

    ## Get-FileHash -Algorithm SHA256 -Path .\Assets\Plugins\spicytech.log
    ##  1ADD6F40CCD914DBA12FB159802E93DED9B83FE5CFAFE78C15903AB882A6E0E1
    Verify-FileHash `
    -FilePath "spicytech.log" `
    -ExpectedHash "C4D3EE680C38E91DA7CFD38E32C480549DFF556C4F5C6D5DECF9A2DE9C76A3F0"

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

$SNIPPETS_DIR="F:\z2025_1\PhysX\PhysX50\physx\snippets"
$PHYSX_COMPILATION_DIR="F:\z2025_1\PhysX\PhysX50\physx\PhysX50"
COMPILATION -FolderName "$SNIPPETS_DIR\Unity"

Set-Location -Path ${EXECUTION_PATH} 
Write-Host "'" ($MyInvocation.MyCommand.Name) "' ..." 

<#---------------------------------------------------------------------------------------------#>