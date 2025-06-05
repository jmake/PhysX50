#pragma once 


#include "PxPhysicsAPI.h"
#include "../snippetcommon/SnippetPrint.h"
#include "../snippetcommon/SnippetPVD.h"
#include "../snippetdeformablevolume/MeshGenerator.h"
#include "extensions/PxTetMakerExt.h"
#include "extensions/PxDeformableVolumeExt.h"

#include "PxDeformableSkinning.h"
#include "gpu/PxPhysicsGpu.h"
#include "extensions/PxCudaHelpersExt.h"
#include "extensions/PxDeformableSkinningExt.h"
#include "extensions/PxRemeshingExt.h"

using namespace physx;
using namespace physx::Ext;
using namespace meshgenerator;

static std::vector<PxVec3>* gVertexBuffer = NULL;


static void releaseVertexBuffer()
{
	if(gVertexBuffer)
	{
		delete gVertexBuffer;
		gVertexBuffer = NULL;
	}
}


static PX_FORCE_INLINE const PxVec3* getVertexBuffer()
{
	PX_ASSERT(gVertexBuffer);
	return &(*gVertexBuffer)[0];
}


static PX_FORCE_INLINE void prepareVertexBuffer()
{
	if(!gVertexBuffer)
		gVertexBuffer = new std::vector<PxVec3>;
	gVertexBuffer->clear();
}


static PX_FORCE_INLINE void pushVertex(const PxVec3& v0, const PxVec3& v1, const PxVec3& v2, const PxVec3& n)
{
	PX_ASSERT(gVertexBuffer);
	gVertexBuffer->push_back(n);	gVertexBuffer->push_back(v0);
	gVertexBuffer->push_back(n);	gVertexBuffer->push_back(v1);
	gVertexBuffer->push_back(n);	gVertexBuffer->push_back(v2);
}



static void renderDeformableVolumeGeometry(const PxTetrahedronMesh& mesh, const PxVec4* deformedPositionsInvMass) 
{
	const int tetFaces[4][3] = { {0,2,1}, {0,1,3}, {0,3,2}, {1,2,3} };

	//Get the deformed vertices. const PxVec3* vertices = mesh.getVertices(); ?? 
	const PxU32 tetCount = mesh.getNbTetrahedrons();
	const PxU32 has16BitIndices = mesh.getTetrahedronMeshFlags() & PxTetrahedronMeshFlag::e16_BIT_INDICES;
	const void* indexBuffer = mesh.getTetrahedrons();

	prepareVertexBuffer();

	const PxU32* intIndices = reinterpret_cast<const PxU32*>(indexBuffer);
	const PxU16* shortIndices = reinterpret_cast<const PxU16*>(indexBuffer);
	PxU32 numTotalTriangles = 0;
	PX_UNUSED(numTotalTriangles);

std::cout<<"[SpicyX] tetCount:" << tetCount <<std::endl;

	for (PxU32 i = 0; i < tetCount; ++i)
	{
		PxU32 vref[4];
		if (has16BitIndices)
		{
			vref[0] = *shortIndices++;
			vref[1] = *shortIndices++;
			vref[2] = *shortIndices++;
			vref[3] = *shortIndices++;
		}
		else
		{
			vref[0] = *intIndices++;
			vref[1] = *intIndices++;
			vref[2] = *intIndices++;
			vref[3] = *intIndices++;
		}

		for (PxU32 j = 0; j < 4; ++j)
		{
            // deformedPositionsInvMass == mPositionsInvMass (from gpu, see 'copyDeformedVerticesFromGPU')
			const PxVec4& v0 = deformedPositionsInvMass[vref[tetFaces[j][0]]];
			const PxVec4& v1 = deformedPositionsInvMass[vref[tetFaces[j][1]]];
			const PxVec4& v2 = deformedPositionsInvMass[vref[tetFaces[j][2]]];

			PxVec3 fnormal = (v1.getXYZ() - v0.getXYZ()).cross(v2.getXYZ() - v0.getXYZ());
			fnormal.normalize();

			pushVertex(v0.getXYZ(), v1.getXYZ(), v2.getXYZ(), fnormal);
			numTotalTriangles++;
		}
	}

	const PxVec3* vertexBuffer = getVertexBuffer();
	int nVertexBuffer  = gVertexBuffer->size();
    std::cout<<"[SpicyX] nVertexBuffer:" << nVertexBuffer <<std::endl;

	//for (int i = 0; i < nVertexBuffer; ++i)printf("Vec%d: (%.2f, %.2f, %.2f)\n", i, vertexBuffer[i].x, vertexBuffer[i].y, vertexBuffer[i].z);

    int nbVertices = mesh.getNbVertices(); 
    std::cout<<"[SpicyX] nVertexBuffer:" << nbVertices <<std::endl;

std::vector<float> flatArray(nbVertices * 4);
std::memcpy(flatArray.data(), deformedPositionsInvMass, nbVertices * sizeof(physx::PxVec4));

static_assert(sizeof(physx::PxVec4) == sizeof(float) * 4, "PxVec4 must be 4 floats.");
const float* flatPtr = reinterpret_cast<const float*>(deformedPositionsInvMass);
//std::vector<float> flatVec(flatPtr, flatPtr + count);

}


void renderDeformableVolume(PxDeformableVolume* deformableVolume, const PxVec4* mpositionsinvmass, int itime)
{

	PxShape* shape = deformableVolume->getShape();

	const PxMat44 shapePose(PxIdentity); // (PxShapeExt::getGlobalPose(*shapes[j], *actors[i]));
	const PxGeometry& geom = shape->getGeometry();

	const PxTetrahedronMeshGeometry& tetGeom = static_cast<const PxTetrahedronMeshGeometry&>(geom);

	const PxTetrahedronMesh& mesh = *tetGeom.tetrahedronMesh; 

	renderDeformableVolumeGeometry(mesh, mpositionsinvmass);
} 


PxU32 DeformableVolumeGetConnectivity(PxDeformableVolume* deformableVolume, std::vector<int>& triangles)
{

	PxShape* shape = deformableVolume->getShape();

	const PxMat44 shapePose(PxIdentity); // (PxShapeExt::getGlobalPose(*shapes[j], *actors[i]));
	const PxGeometry& geom = shape->getGeometry();

	const PxTetrahedronMeshGeometry& tetGeom = static_cast<const PxTetrahedronMeshGeometry&>(geom);

	const PxTetrahedronMesh& mesh = *tetGeom.tetrahedronMesh; 

	//renderDeformableVolumeGeometry(mesh, mpositionsinvmass);
	const PxU32 tetCount = mesh.getNbTetrahedrons();
	const PxU32 has16BitIndices = mesh.getTetrahedronMeshFlags() & PxTetrahedronMeshFlag::e16_BIT_INDICES;
	const void* indexBuffer = mesh.getTetrahedrons();

	const PxU32* intIndices = reinterpret_cast<const PxU32*>(indexBuffer);
	const PxU16* shortIndices = reinterpret_cast<const PxU16*>(indexBuffer);

	PxU32 numTotalTriangles = 0;
	PX_UNUSED(numTotalTriangles);

std::cout<<"[SpicyX] nbTetrahedrons:" << tetCount <<std::endl;

    const int nFaces = 4; 
	const int tetFaces[nFaces][3] = { {0,2,1}, {0,1,3}, {0,3,2}, {1,2,3} };
	for (PxU32 i = 0; i < tetCount; ++i)
	{
		PxU32 vref[nFaces];
		if (has16BitIndices)
		{
			vref[0] = *shortIndices++;
			vref[1] = *shortIndices++;
			vref[2] = *shortIndices++;
			vref[3] = *shortIndices++;
		}
		else
		{
			vref[0] = *intIndices++;
			vref[1] = *intIndices++;
			vref[2] = *intIndices++;
			vref[3] = *intIndices++;
		}

		for (PxU32 j = 0; j < nFaces; ++j)
		{
            // deformedPositionsInvMass == mPositionsInvMass (from gpu, see 'copyDeformedVerticesFromGPU')
			int index1 = vref[tetFaces[j][0]];
			int index2 = vref[tetFaces[j][1]];
			int index3 = vref[tetFaces[j][2]];
            triangles.push_back( index1 ); 
            triangles.push_back( index2 ); 
            triangles.push_back( index3 ); 

//printf("Tetra:%d, Face %d: [%d, %d, %d]\n", i, j, index1, index2, index3);
			numTotalTriangles++;
        }
    } 

    return numTotalTriangles; 
}