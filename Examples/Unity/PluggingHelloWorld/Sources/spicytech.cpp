#include <cuda_runtime.h>

#include "spicytech.hpp"
#include "spicyphysics.hpp"


float PhysxVersion() 
{
  float version = PX_PHYSICS_VERSION_MAJOR + 
           PX_PHYSICS_VERSION_MINOR * 0.1f + 
           PX_PHYSICS_VERSION_BUGFIX * 0.01f;

  std::cout << "[SpicyTech] PhysX Version: "<< version << std::endl;
  return version; 
}


int CudaVersion() 
{
  int runtimeVersion = 0;
  cudaRuntimeGetVersion(&runtimeVersion);  
  std::cout << "[SpicyTech] CUDA Version: " 
            << runtimeVersion / 1000 << "." 
            << (runtimeVersion % 1000) / 10 << std::endl;

  return runtimeVersion; 
}
