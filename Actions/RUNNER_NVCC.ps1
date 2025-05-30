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
function CUDART_SETUP 
{
    param([string]$Root, [string]$Version) 

    echo "[CUDART_SETUP] ..."

    ## 1.0 
    $output="cudart.zip"
    if(Test-Path $output){Remove-Item -Recurse -Force $output}

    $CUDART="cuda_cudart-windows-x86_64-${Version}-archive"
    if(Test-Path $CUDART){Remove-Item -Recurse -Force $CUDART}

    $url="${URL_MAIN}/cuda_cudart/windows-x86_64/$CUDART.zip" 

    Invoke-WebRequest -Uri $url -OutFile $output
    Expand-Archive $output -DestinationPath .   

    ## 2.0 
    Set-Location -Path $CUDART
    Get-ChildItem 

    $env:CUDART_PATH="$Root\$CUDART"
    $env:CUDART_LIB="$env:CUDART_PATH\lib\x64"
    $env:CUDART_INC="$env:CUDART_PATH\include"      

    $env:CUDART_PATH=("$env:CUDART_PATH").replace("\","\\")
    $env:CUDART_LIB=("$env:CUDART_LIB").replace("\","\\")
    $env:CUDART_INC=("$env:CUDART_INC").replace("\","\\")

    $env:CUDART_LIB
    $env:CUDART_INC

    Set-Location -Path $Root
}


function NVCC_SETUP
{ 
    param([string]$Root, [string]$Version) 

    echo "[NVCC_SETUP] ..."

    ## 1.0 
    $output="cudart.zip"
    if(Test-Path $output){Remove-Item -Recurse -Force $output}

    $NVCC="cuda_nvcc-windows-x86_64-${Version}-archive"
    if(Test-Path $NVCC){Remove-Item -Recurse -Force $NVCC}

    $url="${URL_MAIN}/cuda_nvcc/windows-x86_64/${NVCC}.zip"
    
    Invoke-WebRequest -Uri $url -OutFile $output
    Expand-Archive $output -DestinationPath .   
    
 
    ## 2.0 
    Set-Location -Path $NVCC 
    Set-Location bin 
    Get-ChildItem 
    Get-ChildItem nvcc.exe 

    $env:NVCC_PATH="$Root\$NVCC\bin" 
    $env:NVCC="$env:NVCC_PATH\nvcc.exe"  
    
    $env:NVCC_PATH=("$env:NVCC_PATH").replace("\","\\")
    $env:NVCC=("$env:NVCC").replace("\","\\")

    &$env:NVCC --version 

    Set-Location -Path $Root
}


<#---------------------------------------------------------------------------------------------#>
function CreateFile 
{
    param([string]$Path = ".", [string]$FileName, [string]$Content)

    $file = Join-Path $Path $FileName 
    Set-Content -Path $file -Value $Content -Encoding UTF8
    Write-Host "'${file}' created"
}


function SimplestCU_Create 
{
    param([string]$Path = ".")

    $text = @'
#include <stdio.h>

__global__
void cuda_hello()
{
  printf(">> [smallest] Hello from GPU!\n");
}

int main()
{
    printf(">> [smallest] Hello from CPU!\n");   
    cuda_hello<<<1,1>>>();
    return 0;
}
'@
    CreateFile -Path "." -FileName "simplest.cu" -Content $text 
}


function CMakeLists_Create 
{
    $text = @'
cmake_minimum_required(VERSION 3.17 FATAL_ERROR)
project(simplest LANGUAGES CXX CUDA)

add_executable(simplest ./simplest.cu)
set_target_properties(simplest PROPERTIES LINKER_LANGUAGE CXX)

## CMAKE_CUDA_ARCHITECTURES now detected for NVCC, empty CUDA_ARCHITECTURES not allowed.
set_property(TARGET simplest PROPERTY CUDA_ARCHITECTURES OFF) 

enable_testing()
add_test(NAME simplest COMMAND simplest) 
'@
    CreateFile -Path "." -FileName "CMakeLists.txt" -Content $text 
}


<#---------------------------------------------------------------------------------------------#>
function COMPILATION_OBVIOUS 
{ 
    $env:NVCC
    $env:CUDART_LIB
    $env:CUDART_INC

    SimplestCU_Create

    &$env:NVCC -o simplest.exe ./simplest.cu -I"$env:CUDART_INC" -L"$env:CUDART_LIB"
    .\simplest.exe
    rm simplest.exe 
}


function COMPILATION_NICEST 
{ 
    $env:INCLUDES=" -I${env:CUDART_INC}"
    $env:LIBRARIES=" -LIBPATH:${env:CUDART_LIB} "

    SimplestCU_Create
    &$env:NVCC -o simplest.exe ./simplest.cu 
    .\simplest.exe
    rm simplest.exe 

    $env:PATH="$env:PATH;$env:NVCC_PATH;"
    nvcc.exe --version 

    nvcc.exe -o simplest.exe ./simplest.cu
    .\simplest.exe
    rm simplest.exe       
}


function COMPILATION_PERFECT  
{
    $env:INCLUDES=" -I${env:CUDART_INC}"
    $env:LIBRARIES=" -LIBPATH:${env:CUDART_LIB} "
    $env:PATH="$env:PATH;$env:NVCC_PATH;"

    $TEST_PATH="Exec" 
    if (Test-Path $TEST_PATH) {Remove-Item -Recurse -Force $TEST_PATH}
    New-Item -ItemType Directory $TEST_PATH
    Set-Location $TEST_PATH 

    SimplestCU_Create
    CMakeLists_Create 

    cmake.exe . -G Ninja 
    ninja.exe
    ctest.exe
}

<#---------------------------------------------------------------------------------------------#>
$EXECUTION_PATH=(Get-Location).Path 
Write-Host "[EXECUTION_PATH]:'${EXECUTION_PATH}'" 

$RELEASE="11.5.50" # 2022
$RELEASE="12.6.77" # 2025 
$URL_MAIN="https://developer.download.nvidia.com/compute/cuda/redist" 

CL_SETUP
NVCC_SETUP -Root ${EXECUTION_PATH} -Version ${RELEASE}  
CUDART_SETUP -Root ${EXECUTION_PATH} -Version ${RELEASE}  

#COMPILATION_OBVIOUS 
#COMPILATION_NICEST
COMPILATION_PERFECT 

Set-Location -Path ${EXECUTION_PATH} 
Write-Host "'" ($MyInvocation.MyCommand.Name) "' ..." 

<#---------------------------------------------------------------------------------------------#>
