
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
#include <extensions/PxDefaultCpuDispatcher.h>
#include <extensions/PxDefaultSimulationFilterShader.h>

using namespace physx;

static PxPhysics*				gPhysics	= NULL;
static PxFoundation*			gFoundation = NULL;

static PxDefaultAllocator		gAllocator;
static PxDefaultErrorCallback	gErrorCallback;

void InitializePhysX()
{
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);

    std::cout<<"[InitializePhysX]"<<std::endl;
}


void ShutdownPhysX()
{
    gPhysics->release();
    gFoundation->release();

    std::cout<<"[ShutdownPhysX]"<<std::endl;
}


int main()
{
    std::ofstream out("test_output.txt");
    out << "[snippetHelloWorld] ok!" << std::endl;
    std::cout << "[snippetHelloWorld] executed!" << std::endl;

    InitializePhysX();
    ShutdownPhysX();
    return 0;
}
