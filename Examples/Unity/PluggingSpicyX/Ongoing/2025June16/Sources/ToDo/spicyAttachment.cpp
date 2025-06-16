#include <ctype.h>
#include <fstream>
#include <iostream>
#include <cstring>    // for std::memcpy

#include "PxPhysicsAPI.h"
/*
#include "../snippetcommon/SnippetPrint.h"
#include "../snippetcommon/SnippetPVD.h"
#include "../snippetutils/SnippetUtils.h"
//#include "../snippetdeformablevolumeskinning/SnippetDeformableVolumeSkinning.h"
//#include "../snippetdeformablevolume/MeshGenerator.h"
#include "extensions/PxTetMakerExt.h"

#include "PxDeformableSkinning.h"
#include "gpu/PxPhysicsGpu.h"

#include "volumeSkinning.h"
#include "volumeRender.hpp"
#include "task/PxTask.h"

#include "Logger.hpp"
*/

#include "extensions/PxCudaHelpersExt.h"
#include "extensions/PxDeformableSkinningExt.h"
#include "extensions/PxRemeshingExt.h"
#include "extensions/PxDeformableVolumeExt.h"

using namespace physx;
using namespace physx::Ext;

/*
PxRigidDynamic* sphere;

static void initObstacles()
{
	PxShape* shape = gPhysics->createShape(PxSphereGeometry(3.0f), *gMaterial);
	sphere = gPhysics->createRigidDynamic(PxTransform(PxVec3(0.f, 5.0f, 0.f)));
	sphere->attachShape(*shape);
	sphere->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
	gScene->addActor(*sphere);
	shape->release();
}


void stepPhysics()
{
    const PxReal speed = 2.0f;
    PxTransform pose = sphere->getGlobalPose();			
    sphere->setKinematicTarget(PxTransform(pose.p, PxQuat(PxCos(simTime*speed), PxVec3(0,1,0))));
}
*/

// snippets\snippetdeformablevolumeattachment\SnippetDeformableVolumeAttachment.cpp
static PxRigidDynamic* createRigidCube(
    PxPhysics* gPhysics, 
    PxMaterial* gMaterial, 
    PxScene* gScene, 
    PxReal halfExtent, const PxVec3& position)
{
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfExtent, halfExtent, halfExtent), *gMaterial);
	shape->setSimulationFilterData(PxFilterData(0, 0, 1, 0));
	PxTransform localTm(position);

	PxRigidDynamic* body = gPhysics->createRigidDynamic(localTm);
	body->attachShape(*shape);
	PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
	gScene->addActor(*body);
	shape->release();

	return body;
}

void convertCollisionToSim(PxDeformableVolume* deformableVolume, PxU32* tetId, PxVec4* barycentric, PxU32 size)
{
	for (PxU32 i = 0; i < size; i++)
	{
		PxDeformableVolumeExt::convertCollisionToSimulationTet(*deformableVolume, tetId[i], barycentric[i], tetId[i], barycentric[i]);
	}
}

static void connectCubeToDeformableVolume(
    PxPhysics* gPhysics, 
    PxRigidDynamic* cube, PxReal cubeHalfExtent, const PxVec3& cubePosition, PxDeformableVolume* deformableVolume, PxU32 pointGridResolution = 10)
{
	PxArray<PxU32> tetArray;
	PxArray<PxVec4> baryArray;
	PxArray<PxVec4> posArray;

	float f = 2.0f * cubeHalfExtent / (pointGridResolution - 1);
	for (PxU32 ix = 0; ix < pointGridResolution; ++ix)
	{
		PxReal x = ix * f - cubeHalfExtent;
		for (PxU32 iy = 0; iy < pointGridResolution; ++iy)
		{
			PxReal y = iy * f - cubeHalfExtent;
			for (PxU32 iz = 0; iz < pointGridResolution; ++iz)
			{
				PxReal z = iz * f - cubeHalfExtent;
				PxVec3 p(x, y, z);
				PxVec4 bary;
				PxI32 tet = PxTetrahedronMeshExt::findTetrahedronContainingPoint(deformableVolume->getCollisionMesh(), p + cubePosition, bary);
				if (tet >= 0)
				{
					tetArray.pushBack(tet);
					baryArray.pushBack(bary);
					posArray.pushBack(PxVec4(p, 0.0f));
				}
			}
		}
	}

	{
		PxDeformableAttachmentData desc;

		desc.actor[0] = deformableVolume;
		desc.type[0] = PxDeformableAttachmentTargetType::eTETRAHEDRON;
		convertCollisionToSim(deformableVolume, tetArray.begin(), baryArray.begin(), tetArray.size());
		desc.indices[0].data = tetArray.begin();
		desc.indices[0].count = tetArray.size();
		desc.coords[0].data = baryArray.begin();
		desc.coords[0].count = baryArray.size();

		desc.actor[1] = cube;
		desc.type[1] = PxDeformableAttachmentTargetType::eRIGID;
		desc.coords[1].data = posArray.begin();
		desc.coords[1].count = posArray.size();

		gPhysics->createDeformableAttachment(desc);
	}
}

/*
F:\z2025_1\PhysX\PhysX50\physx\snippets\snippetspatialtendon\SnippetSpatialTendon.cpp
F:\z2025_1\PhysX\PhysX50\physx\snippets\snippetpbdcloth\SnippetPBDCloth.cpp
F:\z2025_1\PhysX\PhysX50\physx\snippets\snippetimmediatemode\SnippetImmediateMode.cpp
F:\z2025_1\PhysX\PhysX50\physx\snippets\snippetisosurface\SnippetIsosurface.cpp
*/