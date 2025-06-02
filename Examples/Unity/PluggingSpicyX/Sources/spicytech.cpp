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


SpicyX::SpicyX(int x) : value(x) {}

int SpicyX::GetValue() const 
{
  return value;
}

void SpicyX::SetValue(int x) 
{
  value = x;
}

std::vector<float>& SpicyX::GetFlatArray() 
{
  return flatArray;  // SWIG copies it into a new C# float[]
}

void SpicyX::GetFlatArrayRaw(float* outArray) const {
    std::copy(flatArray.begin(), flatArray.end(), outArray);
}

int SpicyX::GetFlatArraySize() const {
    return static_cast<int>(flatArray.size());
}

void SpicyX::InitFlatArray(int n){
  flatArray = std::vector<float>(n, 0.0f);
}