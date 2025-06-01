#include "spicytech.hpp"
#include <PxConfig.h>
#include <PxPhysicsAPI.h>
#include <cuda_runtime.h>


/* copy the contents of the first array to the second */
void myArrayCopy( int* sourceArray, int* targetArray, int nitems ) {
  int i;
  for ( i = 0; i < nitems; i++ ) {
    targetArray[ i ] = sourceArray[ i ];
  }
}

/* swap the contents of the two arrays */
void myArraySwap( int* array1, int* array2, int nitems ) {
  int i, temp;
  for ( i = 0; i < nitems; i++ ) {
    temp = array1[ i ];
    array1[ i ] = array2[ i ];
    array2[ i ] = temp;
  }
}


void myArrayPrint(int* array1, int nitems)
{
  std::vector<int> v(array1, array1+nitems); 

  //for (int x : v) std::cout << x << " "; std::cout << std::endl;

  std::cout <<" [ ";
  std::copy(v.begin(), v.end(), std::ostream_iterator<int>(std::cout, " "));
  std::cout <<"] "<<std::endl;
}


void PhysxVersion() 
{
  std::cout << "PhysX Version: " 
            << PX_PHYSICS_VERSION_MAJOR << "." 
            << PX_PHYSICS_VERSION_MINOR << "." 
            << PX_PHYSICS_VERSION_BUGFIX << std::endl;
}


void CudaVersion() 
{
  int runtimeVersion = 0;
  cudaRuntimeGetVersion(&runtimeVersion);  
  std::cout << "CUDA Version: " 
            << runtimeVersion / 1000 << "." 
            << (runtimeVersion % 1000) / 10 << std::endl;
}
