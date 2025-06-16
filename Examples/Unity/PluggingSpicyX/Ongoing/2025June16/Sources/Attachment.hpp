#pragma once
#include <vector>
#include <iostream>

#include <iterator>
#include <algorithm>

#include <ctype.h>
#include <fstream>
#include <iostream>
#include <cstring>    // for std::memcpy
#include <sstream>


#include "PxPhysicsAPI.h"

using namespace physx;

#include "Logger.hpp"


//-----------------------------------------------------------------//
//-----------------------------------------------------------------//
void convertCollisionToSim(PxDeformableVolume* deformableVolume, PxU32* tetId, PxVec4* barycentric, PxU32 size)
{
	for (PxU32 i = 0; i < size; i++)
	{
		PxDeformableVolumeExt::convertCollisionToSimulationTet(*deformableVolume, tetId[i], barycentric[i], tetId[i], barycentric[i]);
	}
}

/*
static void createDeformableVolumes(const PxCookingParams& params)
{
    ...
	PxReal halfExtent = 1;
	PxVec3 cubePosA(0, 7.25, 0);
	PxVec3 cubePosB(0, 11.75, 0);
	PxRigidDynamic* rigidCubeA = createRigidCube(halfExtent, cubePosA);
	PxRigidDynamic* rigidCubeB = createRigidCube(halfExtent, cubePosB);
	
	connectCubeToDeformableVolume(rigidCubeA, 2*halfExtent, cubePosA, deformableVolumeSphere);
	connectCubeToDeformableVolume(rigidCubeA, 2*halfExtent, cubePosA, deformableVolumeCube);

	connectCubeToDeformableVolume(rigidCubeB, 2*halfExtent, cubePosB, deformableVolumeCube);
	connectCubeToDeformableVolume(rigidCubeB, 2*halfExtent, cubePosB, deformableVolumeCone);
}
*/

/*
static void connectCubeToDeformableVolume(
    PxPhysics* physics, 
    PxRigidDynamic* cube, 
    PxReal cubeHalfExtent, 
    const PxVec3& cubePosition, 
    PxDeformableVolume* deformableVolume, 
    PxU32 pointGridResolution = 10)
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

		physics->createDeformableAttachment(desc);
	}
}
*/




//-----------------------------------------------------------------//
//-----------------------------------------------------------------//










//-----------------------------------------------------------------//
//-----------------------------------------------------------------//