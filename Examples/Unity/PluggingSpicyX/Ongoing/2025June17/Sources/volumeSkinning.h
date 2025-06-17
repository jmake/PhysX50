#include <ctype.h>
#include <fstream>
#include <iostream>

#include <cstdio>
#include <vector>
#include <cstring>    // for std::memcpy

#ifndef PHYSX_SNIPPET_DEFORMABLE_VOLUME_SKINNING_H
#define PHYSX_SNIPPET_DEFORMABLE_VOLUME_SKINNING_H

#include "PxPhysicsAPI.h"
#include "cudamanager/PxCudaContextManager.h"
#include "cudamanager/PxCudaContext.h"
#include "extensions/PxCudaHelpersExt.h"

struct SkinnedMesh
{
	physx::PxArray<physx::PxVec3> mVertices;
	physx::PxArray<physx::PxU32> mTriangles;
};

struct BasePostSolveCallback : physx::PxPostSolveCallback
{
	virtual void synchronize() = 0;
	virtual physx::PxVec3* getSkinnedVertices(physx::PxU32 deformableVolumeIndex) = 0;
};

class DeformableVolume
{
public:
	DeformableVolume() :
		mPositionsInvMass(NULL),
		mDeformableVolume(NULL),
		mCudaContextManager(NULL)
	{ }

	DeformableVolume(physx::PxDeformableVolume* deformableVolume, physx::PxCudaContextManager* cudaContextManager) :
		mDeformableVolume(deformableVolume),
		mCudaContextManager(cudaContextManager)
	{
		mPositionsInvMass = PX_EXT_PINNED_MEMORY_ALLOC(physx::PxVec4, *cudaContextManager, deformableVolume->getCollisionMesh()->getNbVertices());
	}

	~DeformableVolume()
	{
	}

	void release()
	{
		if (mDeformableVolume)
			mDeformableVolume->release();

		PX_EXT_PINNED_MEMORY_FREE(*mCudaContextManager, mPositionsInvMass);
	}

	void copyDeformedVerticesFromGPUAsync(CUstream stream)
	{
		physx::PxTetrahedronMesh* tetMesh = mDeformableVolume->getCollisionMesh();

		physx::PxScopedCudaLock _lock(*mCudaContextManager);
		mCudaContextManager->getCudaContext()->memcpyDtoHAsync(mPositionsInvMass, reinterpret_cast<CUdeviceptr>(mDeformableVolume->getPositionInvMassBufferD()), tetMesh->getNbVertices() * sizeof(physx::PxVec4), stream);
	}


	std::vector<float> copyDeformedVerticesFromGPU(physx::PxU32 iteration, physx::PxU32 ibody)
	{
//std::cout<<"[SpicyTech] copyDeformedVerticesFromGPU"<<std::endl;

		physx::PxTetrahedronMesh* tetMesh = mDeformableVolume->getCollisionMesh();

		physx::PxScopedCudaLock _lock(*mCudaContextManager);
		mCudaContextManager->getCudaContext()->memcpyDtoH(mPositionsInvMass, 
														  reinterpret_cast<CUdeviceptr>(mDeformableVolume->getPositionInvMassBufferD()), 
														  tetMesh->getNbVertices() * sizeof(physx::PxVec4)
														);

		physx::PxU32 nbVertices = tetMesh->getNbVertices(); 

std::vector<float> flatArray(nbVertices * 4);
std::memcpy(flatArray.data(), mPositionsInvMass, nbVertices * sizeof(physx::PxVec4));
/*
bool equal = (std::memcmp(flatArray.data(), mPositionsInvMass, nbVertices * sizeof(physx::PxVec4)) == 0);
if(!equal) 
{
	std::cerr<< "[ERROR] Mismatch between flatArray and mPositionsInvMass!"<< std::endl;
	exit(1); 
}

if(iteration==0 && ibody==0)
{
	if( abs(flatArray[4*0 + 0] - 1.25    ) > 1e-3 || 
		abs(flatArray[4*0 + 1] - 7.747282) > 1e-3 || 
		abs(flatArray[4*0 + 2] - -1.25   ) > 1e-3 || 
		abs(flatArray[4*0 + 3] - 1.572864) > 1e-3 
	  ) 
	{
		std::cerr<< "[ERROR] Mismatch between flatArray and mPositionsInvMass!!"<< std::endl;
		exit(1);
	} 
}
*/
/*
for (int i = 0; i < flatArray.size() / 4; ++i)
{
    float* v = &flatArray[i * 4];
    printf("Vertex %u: %f %f %f %f\n", i, v[0], v[1], v[2], v[3]);
	// Vertex   0: 1.250000 7.747282 -1.250000 1.572864
	// Vertex 556: 0.098849 12.586310 -0.001949 0.080996
}
*/

/*
		for(int i=0; i<nbVertices; i++)
		{
			physx::PxVec4 array(mPositionsInvMass[i]) ;

			std::cout
			<< iteration <<" "
			<< ibody <<" "
			<< i <<" "
			<< array[0] <<" "
			<< array[1] <<" "
			<< array.z <<" "
			<< array.w <<" "
			<<" ("<< flatArray[4*i + 0] <<" "<< flatArray[4*i + 1] <<" "<< flatArray[4*i + 2] <<" "<< flatArray[4*i + 3] <<" "<<") "
			<< std::endl;
		} 	
*/
		return flatArray; 
	}


	physx::PxVec4* mPositionsInvMass;
	physx::PxDeformableVolume* mDeformableVolume;
	physx::PxCudaContextManager* mCudaContextManager;
};

#endif
