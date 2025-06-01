#include "spicytech.hpp"

#include <cuda_runtime.h>

#include <PxConfig.h>
#include <PxPhysicsAPI.h>

#include <foundation/PxMat33.h>
#include <extensions/PxShapeExt.h>
#include <extensions/PxSimpleFactory.h>
#include <extensions/PxExtensionsAPI.h>
#include <extensions/PxDefaultAllocator.h>
#include <extensions/PxDefaultErrorCallback.h>
//#include <extensions/PxDefaultCpuDispatcher.h>
//#include <extensions/PxDefaultSimulationFilterShader.h>

using namespace physx;

static PxPhysics*  gPhysics	= NULL;
static PxFoundation*  gFoundation = NULL;

static PxDefaultAllocator  gAllocator;
static PxDefaultErrorCallback  gErrorCallback;

void myArrayCopy( int* sourceArray, int* targetArray, int nitems ) 
{
  for (int i = 0; i < nitems; i++ ) targetArray[ i ] = sourceArray[ i ];
}


void myArrayPrint(int* array1, int nitems)
{
  std::vector<int> v(array1, array1+nitems); 

  std::cout <<" [ ";
  std::copy(v.begin(), v.end(), std::ostream_iterator<int>(std::cout, " "));
  std::cout <<"] "<<std::endl;
}


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
