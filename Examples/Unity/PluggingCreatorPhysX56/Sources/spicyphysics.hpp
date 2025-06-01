#include <fstream>
#include <iostream>

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

#include <cuda_runtime.h>

using namespace physx;

static PxPhysics*  gPhysics	= NULL;
static PxFoundation*  gFoundation = NULL;

static PxDefaultAllocator  gAllocator;
static PxDefaultErrorCallback  gErrorCallback;


void SpicyShutdown()
{
    if(gPhysics) gPhysics->release();
    if(gFoundation) gFoundation->release();

    std::cout<<"[SpicyX] Shutdown"<<std::endl;
}

void SpicyInitialize()
{
    gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
    std::cout<<"[SpicyX] Initialize"<<std::endl;
}


int SpicyTest()
{
    SpicyInitialize(); 
    SpicyShutdown(); 
    std::cout<<"[SpicyX] Test"<<std::endl;

    return 1; 
}