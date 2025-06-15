#include <ctype.h>
#include <fstream>
#include <iostream>
#include <cstring>    // for std::memcpy

#include "PxPhysicsAPI.h"
#include "../snippetcommon/SnippetPrint.h"
#include "../snippetcommon/SnippetPVD.h"
#include "../snippetutils/SnippetUtils.h"
//#include "../snippetdeformablevolumeskinning/SnippetDeformableVolumeSkinning.h"
#include "../snippetdeformablevolume/MeshGenerator.h"
#include "extensions/PxTetMakerExt.h"
#include "extensions/PxDeformableVolumeExt.h"

#include "PxDeformableSkinning.h"
#include "gpu/PxPhysicsGpu.h"
#include "extensions/PxCudaHelpersExt.h"
#include "extensions/PxDeformableSkinningExt.h"
#include "extensions/PxRemeshingExt.h"

#include "volumeSkinning.h"
#include "volumeRender.hpp"
#include "task/PxTask.h"

#include "JsPhysX/triangle_mesh_create.hpp"
#include "Attachment.hpp"
#include "RigidBodyKinematic.hpp"
#include "Logger.hpp"


#define MAX_NUM_ACTOR_SHAPES	128

using namespace physx;
using namespace physx::Ext;
using namespace meshgenerator;

//extern PxArray<DeformableSurface> gDeformableSurfaces; // :( 

static PxDefaultAllocator		gAllocator;
static PxDefaultErrorCallback	gErrorCallback;
static PxFoundation* gFoundation = NULL;
static PxPhysics* gPhysics = NULL;
static PxCudaContextManager* gCudaContextManager = NULL;
static PxDefaultCpuDispatcher* gDispatcher = NULL;
static PxScene* gScene = NULL;
static PxMaterial* gMaterial = NULL;
static PxPvd* gPvd = NULL;
PxArray<DeformableVolume> gDeformableVolumes;
PxArray<SkinnedMesh> gSkinnedMeshes;
BasePostSolveCallback* gSkinning;

//-----------------------------------------------------------------//
//-----------------------------------------------------------------//
static int itime = 0; 
bool interactive = true; // -> 'True' during Rendering 


std::vector< float* > UnitySoftVerts; 
std::vector< int > UnitySoftnVerts; 

std::vector< int* > UnitySoftIndices; 
std::vector< int > UnitySoftnIndices; 

Logger* logger = nullptr;
//RigidBodyKinematic* rbkObj = nullptr; 

const PxReal deltaTime = 1.0f / 60.f;


//-----------------------------------------------------------------//
//-----------------------------------------------------------------//
static bool			gPause			= false;
static bool			gOneFrame		= false;
static const PxU32	gScenarioCount	= 2;
static PxU32		gScenario		= 0;

PxTolerancesScale scale;

template<typename T>
class HostAndDeviceBuffer
{
public:
	PxCudaContextManager* mContextManager;
	T* mDeviceData;
	T* mHostData; //Pinned host memory
	PxU32 mNumElements;

	HostAndDeviceBuffer() :
		mContextManager(NULL), mDeviceData(NULL), mHostData(NULL), mNumElements(0)
	{}

	HostAndDeviceBuffer(PxCudaContextManager* contextManager, PxU32 numElements) :
		mContextManager(contextManager), mDeviceData(NULL), mHostData(NULL), mNumElements(0)
	{
		allocate(numElements);
	}

	void initialize(PxCudaContextManager* contextManager, PxU32 numElements)
	{
		mContextManager = contextManager;
		allocate(numElements);
	}

	void initialize(PxCudaContextManager* contextManager, const T* dataSource, PxU32 numElements)
	{
		mContextManager = contextManager;
		allocate(numElements);
		PxMemCopy(mHostData, dataSource, numElements * sizeof(T));
	}

	void allocate(PxU32 numElements)
	{
		release();
		mDeviceData = PxCudaHelpersExt::allocDeviceBuffer<T>(*mContextManager, numElements);
		mHostData = PxCudaHelpersExt::allocPinnedHostBuffer<T>(*mContextManager, numElements);
		mNumElements = numElements;
	}

	void copyDeviceToHost(PxU32 numElementsToCopy = 0xFFFFFFFF)
	{
		PxCudaHelpersExt::copyDToH(*mContextManager, mHostData, mDeviceData, PxMin(numElementsToCopy, mNumElements));
	}

	void copyHostToDevice(PxU32 numElementsToCopy = 0xFFFFFFFF)
	{
		PxCudaHelpersExt::copyHToD<T>(*mContextManager, mDeviceData, mHostData, PxMin(numElementsToCopy, mNumElements));
	}

	void copyDeviceToHostAsync(CUstream stream, PxU32 numElementsToCopy = 0xFFFFFFFF)
	{
		PxCudaHelpersExt::copyDToHAsync(*mContextManager, mHostData, mDeviceData, PxMin(numElementsToCopy, mNumElements), stream);
	}

	void release()
	{
		PxCudaHelpersExt::freeDeviceBuffer(*mContextManager, mDeviceData);
		PxCudaHelpersExt::freePinnedHostBuffer(*mContextManager, mHostData);
	}
};

struct VolumeSkinningHelper
{
	PxDeformableVolume* mDeformableVolume;
	HostAndDeviceBuffer<PxU32> mDeformableVolumeTets;
	HostAndDeviceBuffer<PxTetrahedronMeshEmbeddingInfo> mSkinningInfo;
	HostAndDeviceBuffer<PxVec3> mSkinnedVertices;
	PxU32 mNumSkinnedVertices;

	VolumeSkinningHelper() : mDeformableVolume(NULL), mNumSkinnedVertices(0)
	{ }

	VolumeSkinningHelper(PxCudaContextManager* contextManager, PxDeformableVolume* deformableVolume, PxVec3* skinnedPointsRestPosition, PxU32 nbSkinnedPoints)
		: mDeformableVolume(deformableVolume)
	{
		PxTetrahedronMesh& simulationMesh = *deformableVolume->getSimulationMesh();

		PxU32 nbTetrahedra = simulationMesh.getNbTetrahedrons();

		bool uses16bit = simulationMesh.getTetrahedronMeshFlags() & PxTetrahedronMeshFlag::e16_BIT_INDICES;
		mDeformableVolumeTets.initialize(contextManager, 4 * nbTetrahedra);

		if (uses16bit)
		{
			const PxU16* tetIndices = reinterpret_cast<const PxU16*>(simulationMesh.getTetrahedrons());
			for (PxU32 i = 0; i < mDeformableVolumeTets.mNumElements; ++i)
				mDeformableVolumeTets.mHostData[i] = tetIndices[i];
		}
		else
		{
			const PxU32* tetIndices = reinterpret_cast<const PxU32*>(simulationMesh.getTetrahedrons());
			for (PxU32 i = 0; i < mDeformableVolumeTets.mNumElements; ++i)
				mDeformableVolumeTets.mHostData[i] = tetIndices[i];
		}

		mSkinnedVertices.initialize(contextManager, skinnedPointsRestPosition, nbSkinnedPoints);
		mNumSkinnedVertices = nbSkinnedPoints;
		mSkinningInfo.initialize(contextManager, nbSkinnedPoints);

		PxDeformableSkinningExt::initializeInterpolatedVertices(
			mSkinningInfo.mHostData, simulationMesh.getVertices(), mDeformableVolumeTets.mHostData,
			nbTetrahedra, skinnedPointsRestPosition, nbSkinnedPoints);

		mDeformableVolumeTets.copyHostToDevice();
		mSkinnedVertices.copyHostToDevice();
		mSkinningInfo.copyHostToDevice();
	}

	void packageGpuData(PxTetmeshSkinningGpuData& target)
	{
		target.guideVerticesD.data = reinterpret_cast<PxVec3*>(mDeformableVolume->getSimPositionInvMassBufferD());
		target.guideVerticesD.stride = sizeof(PxVec4);
		target.guideTetrahedraD = mDeformableVolumeTets.mDeviceData;
		target.skinningInfoPerVertexD = mSkinningInfo.mDeviceData;
		target.skinnedVerticesD.count = mNumSkinnedVertices;
		target.skinnedVerticesD.stride = sizeof(PxVec3);
		target.skinnedVerticesD.data = mSkinnedVertices.mHostData; //This works because it's pinned memory - no device to host transfer will be required
	}

	void release()
	{
		mDeformableVolumeTets.release();
		mSkinnedVertices.release();
		mSkinningInfo.release();
	}
};


struct PostSolveCallback : BasePostSolveCallback, PxUserAllocated
{
	CUstream mSkinningStream;
	PxCudaContextManager* mContextManager;
	PxDeformableSkinning* skinning;
	PxArray<VolumeSkinningHelper> skinningHelpers;
	HostAndDeviceBuffer<PxTetmeshSkinningGpuData> packagedSkinningData; 


	PostSolveCallback(PxCudaContextManager* contextManager, PxU32 maxNumDeformableVolumes) :
		mContextManager(contextManager)
	{
		const PxU32 CU_STREAM_NON_BLOCKING = 0x1;
		mContextManager->getCudaContext()->streamCreate(&mSkinningStream, CU_STREAM_NON_BLOCKING);

		skinning = PxGetPhysicsGpu()->createDeformableSkinning(contextManager);

		packagedSkinningData.initialize(contextManager, maxNumDeformableVolumes);
		skinningHelpers.resize(maxNumDeformableVolumes);
	}

	void setDeformableVolume(PxU32 index, PxDeformableVolume* deformableVolume, PxVec3* skinnedPointsRestPosition, PxU32 nbSkinnedPoints)
	{
		skinningHelpers[index] = VolumeSkinningHelper(mContextManager, deformableVolume, skinnedPointsRestPosition, nbSkinnedPoints);
	}

	virtual void onPostSolve(CUevent startEvent)
	{
		mContextManager->getCudaContext()->streamWaitEvent(mSkinningStream, startEvent);

		for (PxU32 i = 0; i < skinningHelpers.size(); ++i) 		
			skinningHelpers[i].packageGpuData(packagedSkinningData.mHostData[i]);
		
		packagedSkinningData.copyHostToDevice(skinningHelpers.size());

		skinning->evaluateVerticesEmbeddedIntoVolume(packagedSkinningData.mDeviceData, skinningHelpers.size(), mSkinningStream);

		//mSkinnedVertices.copyDeviceToHostAsync(mSkinningStream);
	}

	virtual void synchronize()
	{
		mContextManager->getCudaContext()->streamSynchronize(mSkinningStream);
	}

	virtual PxVec3* getSkinnedVertices(PxU32 deformableVolumeIndex)
	{
		return skinningHelpers[deformableVolumeIndex].mSkinnedVertices.mHostData;
	}

	~PostSolveCallback()
	{
		mContextManager->getCudaContext()->streamDestroy(mSkinningStream);
		for (PxU32 i = 0; i < skinningHelpers.size(); ++i)
			skinningHelpers[i].release();
		PX_DELETE(skinning);
	}
};

PostSolveCallback* postSolveCallback;

void addDeformableVolume(PxDeformableVolume* deformableVolume, const PxTransform& transform, const PxReal density, const PxReal scale)
{
	logger->log("[addDeformableVolume] ..."); 

	PxVec4* simPositionInvMassPinned;
	PxVec4* simVelocityPinned;
	PxVec4* collPositionInvMassPinned;
	PxVec4* restPositionPinned;

	PxDeformableVolumeExt::allocateAndInitializeHostMirror(*deformableVolume, gCudaContextManager, simPositionInvMassPinned, simVelocityPinned, collPositionInvMassPinned, restPositionPinned);

	const PxReal maxInvMassRatio = 50.f;

	PxDeformableVolumeExt::transform(*deformableVolume, transform, scale, simPositionInvMassPinned, simVelocityPinned, collPositionInvMassPinned, restPositionPinned);
	PxDeformableVolumeExt::updateMass(*deformableVolume, density, maxInvMassRatio, simPositionInvMassPinned);
	PxDeformableVolumeExt::copyToDevice(*deformableVolume, PxDeformableVolumeDataFlag::eALL, simPositionInvMassPinned, simVelocityPinned, collPositionInvMassPinned, restPositionPinned);

	DeformableVolume volume(deformableVolume, gCudaContextManager);

	gDeformableVolumes.pushBack(volume);

	PX_EXT_PINNED_MEMORY_FREE(*gCudaContextManager, simPositionInvMassPinned);
	PX_EXT_PINNED_MEMORY_FREE(*gCudaContextManager, simVelocityPinned);
	PX_EXT_PINNED_MEMORY_FREE(*gCudaContextManager, collPositionInvMassPinned);
	PX_EXT_PINNED_MEMORY_FREE(*gCudaContextManager, restPositionPinned);

	logger->log("[addDeformableVolume] !!"); 
}


static PxDeformableVolume* 
DeformableVolumeCreate(const PxCookingParams& params, const PxArray<PxVec3>& triVerts, const PxArray<PxU32>& triIndices, bool useCollisionMeshForSimulation = false)
{
	logger->log("[createDeformableVolume] ..."); 

	PxDeformableVolumeMesh* deformableVolumeMesh;

	PxU32 numVoxelsAlongLongestAABBAxis = 8;

	logger->log(
		"[createDeformableVolume] gDeformableVolumes:" + 
		std::to_string( gDeformableVolumes.size() ) +
		" triVerts:" + std::to_string( triVerts.size() ) +
		" triIndices:" + std::to_string( triIndices.size() / 3 ) 
	); 
	
	bool contidion = (triVerts.size() == 0) || (triIndices.size() == 0); 
	logger->error(contidion, 
				"[createDeformableVolume] (triVerts.size() == 0) || (triIndices.size() == 0) -> " + 
				std::to_string(contidion) ); 

	PxSimpleTriangleMesh surfaceMesh;
	surfaceMesh.points.count = triVerts.size();
	surfaceMesh.points.data = triVerts.begin();
	surfaceMesh.triangles.count = triIndices.size() / 3;
	surfaceMesh.triangles.data = triIndices.begin();
	PxTetMaker::validateTriangleMesh(surfaceMesh); 

	if (useCollisionMeshForSimulation)
	{
		deformableVolumeMesh = PxDeformableVolumeExt::createDeformableVolumeMeshNoVoxels(params, surfaceMesh, gPhysics->getPhysicsInsertionCallback());
	}
	else
	{
		deformableVolumeMesh = PxDeformableVolumeExt::createDeformableVolumeMesh(params, surfaceMesh, numVoxelsAlongLongestAABBAxis, gPhysics->getPhysicsInsertionCallback());
	}

	//Alternatively one can cook a deformable volume mesh in a single step
	//tetMesh = cooking.createDeformableVolumeMesh(simulationMeshDesc, collisionMeshDesc, deformableVolumeDesc, physics.getPhysicsInsertionCallback());
	PX_ASSERT(deformableVolumeMesh);

	if (!gCudaContextManager)
		return NULL;
	PxDeformableVolume* deformableVolume = gPhysics->createDeformableVolume(*gCudaContextManager);
	if (deformableVolume)
	{
		PxShapeFlags shapeFlags = PxShapeFlag::eVISUALIZATION | PxShapeFlag::eSCENE_QUERY_SHAPE | PxShapeFlag::eSIMULATION_SHAPE;

		PxDeformableVolumeMaterial* materialPtr = PxGetPhysics().createDeformableVolumeMaterial(2.e+5f, 0.3f, 0.1f);
		PxTetrahedronMeshGeometry geometry(deformableVolumeMesh->getCollisionMesh());
		PxShape* shape = gPhysics->createShape(geometry, &materialPtr, 1, true, shapeFlags);
		if (shape)
		{
			deformableVolume->attachShape(*shape);
			shape->setSimulationFilterData(PxFilterData(0, 0, 2, 0));
		}
		deformableVolume->attachSimulationMesh(*deformableVolumeMesh->getSimulationMesh(), *deformableVolumeMesh->getDeformableVolumeAuxData());

		gScene->addActor(*deformableVolume);

		addDeformableVolume(deformableVolume, PxTransform(PxVec3(0.f, 0.f, 0.f), PxQuat(PxIdentity)), 100.f, 1.0f);
		deformableVolume->setDeformableBodyFlag(PxDeformableBodyFlag::eDISABLE_SELF_COLLISION, true);
		deformableVolume->setSolverIterationCounts(30);


		PxArray<PxU32> subdividedTriangles = triIndices;
		PxArray<PxVec3> subdividedVertices = triVerts;
		PxRemeshingExt::limitMaxEdgeLength(subdividedTriangles, subdividedVertices, 0.0001f, 2);

		SkinnedMesh mesh;
		for (PxU32 i = 0; i < subdividedTriangles.size(); ++i)
			mesh.mTriangles.pushBack(subdividedTriangles[i]);
		for (PxU32 i = 0; i < subdividedVertices.size(); ++i)
			mesh.mVertices.pushBack(subdividedVertices[i]);

		gSkinnedMeshes.pushBack(mesh);
	}

	logger->log("[createDeformableVolume] !!"); 
	return deformableVolume;
}


//-----------------------------------------------------------------//
//-----------------------------------------------------------------//
// MeshCreate(vertices, nVertices * nDims, triangles, nTriangles * nIndices); 
// static_assert(sizeof(physx::PxVec3) == sizeof(float) * 3, "Incompatible layout");
void UnitySoftClear()
{
//    std::cout<<"[SpicyX] UnitySoftClear"<<std::endl;
	logger->log("[UnityClear] ...");

	UnitySoftnVerts.clear(); 
	UnitySoftVerts.clear();
	
	UnitySoftnIndices.clear();
	UnitySoftIndices.clear();

	logger->log("[UnityClear] !!");
}


void UnitySoftAdd(float* outArray1, int n1, int* outArray2, int n2) 
{
	//spicyX.MeshAdd(vertices, vertices.Length, triangles, triangles.Length); 
	UnitySoftnVerts.push_back( n1 ); 
	UnitySoftVerts.push_back( outArray1 ); 

	UnitySoftnIndices.push_back( n2 ); 
	UnitySoftIndices.push_back( outArray2 ); 

	if(logger == NULL) return; 

	logger->log(
		"[UnityInit] " +
		std::to_string(UnitySoftnVerts.size()-1) + ") triVerts:" + std::to_string( n1 / 3 ) +
		" triangles:" + std::to_string( n2 / 3 ) 
	); 	

	logger->error(n1 % 3 != 0, "[ERROR] triVerts % 3 != 0 !!"); 
	logger->error(n2 % 3 != 0, "[ERROR] triangles % 3 != 0 !!"); 

} 


void UnitySoftCreate(
	 PxArray<PxVec3>& uVerts,  
	 PxArray<PxU32>& uIndices, 
	 float* outArray1, int n1, 
	 int* outArray2, int n2
	) 
{
	//spicyX.MeshAdd(vertices, vertices.Length, triangles, triangles.Length); 
	// outArray1 : n1 -> nVertices * nDims
	// outArray2 : n2 -> nTriangles * nIndices 
	int nDims = 3;
	int nVertices = n1 / nDims; 

	//PxArray<PxVec3> uVerts; 
	uVerts.clear(); 
	uVerts.resize(nVertices); // n1 -> nVertices

	for(int i=0,k=0; i < nVertices; i++)
	{
		uVerts[i] = PxVec3(outArray1[nDims*i+0], outArray1[nDims*i+1], outArray1[nDims*i+2]) ; 
		printf("%d) %f %f %f \n", i, uVerts[i][0], uVerts[i][1], uVerts[i][2]);
	}

	int nIndices = 3; 
	int nTriangles = n2 / nIndices; 

	//PxArray<PxU32> uIndices; 
	uIndices.clear(); 
	uIndices.resize(n2);
	for (int i = 0; i < n2; ++i) 
	{
		uIndices[i] = static_cast<PxU32>(outArray2[i]);
		printf("%d) %d \n", i, uIndices[i]);
	}

	logger->log(
		"[UnityCreate] nVertices:" +  
		std::to_string( nVertices ) + " (" + std::to_string( uVerts.size() ) + ")"
		" nTriangles:" + 
		std::to_string( nTriangles ) + " (" + std::to_string( uIndices.size() / nIndices ) + ")"
	);

	logger->error(nVertices != uVerts.size(), "[ERROR] nVertices != uVerts.size() !!"); 
	logger->error(nTriangles != uIndices.size() / 3, "[ERROR] nTriangles != uIndices.size() / 3 !!"); 
}


//-----------------------------------------------------------------//
//-----------------------------------------------------------------//
void TriangleMeshSet(
	std::vector<PxVec3> vertices, 
	std::vector<PxU32> triangles, 
	PxArray<PxVec3>& triVerts, 
	PxArray<PxU32>& triIndices, 
	const PxVec3& pos, 
	PxReal scaling)
{
    triVerts.clear();
    triIndices.clear();

    for (const auto& v : vertices)
        triVerts.pushBack(scaling * v + pos);

    for (const auto& i : triangles)
        triIndices.pushBack(i);
}


//-----------------------------------------------------------------//
void createCube2(
	PxArray<PxVec3>& triVerts, 
	PxArray<PxU32>& triIndices, 
	const PxVec3& pos, 
	PxReal scaling)
{
	std::vector<PxVec3> vertices = {
		{ 0.5f, -0.5f, -0.5f },
		{ 0.5f, -0.5f,  0.5f },
		{-0.5f, -0.5f,  0.5f },
		{-0.5f, -0.5f, -0.5f },
		{ 0.5f,  0.5f, -0.5f },
		{ 0.5f,  0.5f,  0.5f },
		{-0.5f,  0.5f,  0.5f },
		{-0.5f,  0.5f, -0.5f }
	};

	std::vector<PxU32> triangles = {
		1, 2, 3,
		7, 6, 5,
		4, 5, 1,
		5, 6, 2,
		2, 6, 7,
		0, 3, 7,
		0, 1, 3,
		4, 7, 5,
		0, 4, 1,
		1, 5, 2,
		3, 2, 7,
		4, 0, 7
	};

	TriangleMeshSet(vertices, triangles, triVerts, triIndices, pos, scaling); 
} 


//-----------------------------------------------------------------//
//-----------------------------------------------------------------//
static void DeformableVolumesCreate(const PxCookingParams& params)
{
	logger->log("[createDeformableVolumes] ..."); 

	if (gCudaContextManager == NULL)
	{
		printf("The Deformable Volume feature is currently only supported on GPU\n");
		return;
	}

	PxArray<PxVec3> triVerts;
	PxArray<PxU32> triIndices;

	PxReal maxEdgeLength = 1;

	createCube2(triVerts, triIndices, PxVec3(0.0, 9, 0), 2.5);
	PxRemeshingExt::limitMaxEdgeLength(triIndices, triVerts, maxEdgeLength);
	DeformableVolumeCreate(params, triVerts, triIndices);

	createSphere(triVerts, triIndices, PxVec3(0, 4.5, 0), 2.5, maxEdgeLength);
	DeformableVolumeCreate(params, triVerts, triIndices);

	createConeY(triVerts, triIndices, PxVec3(0.1, 11.5, 0), 2.0f, 3.5);
	PxRemeshingExt::limitMaxEdgeLength(triIndices, triVerts, maxEdgeLength);
	DeformableVolumeCreate(params, triVerts, triIndices);

	// MeshCreate 
	printf("[createDeformableVolumes] nDeformables:%zd \n", UnitySoftVerts.size());
	for(int i=0; i < UnitySoftVerts.size(); i++)
	{
		PxArray<PxVec3> uVerts;   
		PxArray<PxU32> uIndices; 
		UnitySoftCreate(
			uVerts, uIndices, 
			UnitySoftVerts[i], UnitySoftnVerts[i], 
			UnitySoftIndices[i], UnitySoftnIndices[i]); 

		PxRemeshingExt::limitMaxEdgeLength(uIndices, uVerts, maxEdgeLength);
		DeformableVolumeCreate(params, uVerts, uIndices);
	}

	// spicyAttachment.hpp

	postSolveCallback = PX_NEW(PostSolveCallback)(gCudaContextManager, PxU32(gSkinnedMeshes.size()));
	gSkinning = postSolveCallback;

	for (PxU32 i = 0; i < gSkinnedMeshes.size(); ++i) 
	{
		SkinnedMesh& skinnedMesh = gSkinnedMeshes[i];
		postSolveCallback->setDeformableVolume(i, gDeformableVolumes[i].mDeformableVolume, &skinnedMesh.mVertices[0], PxU32(skinnedMesh.mVertices.size()));
	}

	gScene->setDeformableVolumeGpuPostSolveCallback(postSolveCallback);

	logger->log("[createDeformableVolumes] !!"); 
}


//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
void rigidBodiesRenderActors(PxRigidActor** actors, const PxU32 numActors, int itime) 
{
	std::vector<int> Active; 

	PxShape* shapes[MAX_NUM_ACTOR_SHAPES];
	for(PxU32 i=0,idx=0; i<numActors; i++)
	{
		const PxU32 nbShapes = actors[i]->getNbShapes();
		PX_ASSERT(nbShapes <= MAX_NUM_ACTOR_SHAPES);
		actors[i]->getShapes(shapes, nbShapes);
		const bool sleeping = actors[i]->is<PxRigidDynamic>() ? actors[i]->is<PxRigidDynamic>()->isSleeping() : false;

		for(PxU32 j=0;j < nbShapes;j++)
		{
			const PxMat44 shapePose(PxShapeExt::getGlobalPose(*shapes[j], *actors[i]));
			const PxGeometryHolder h = shapes[j]->getGeometry();

			if(!itime)
			{
				std::vector< std::vector<int> > faces;
				std::vector< std::vector<double> > vertices;
				GetTriangleMesh(h.any(), vertices, faces);
				vertices.clear();
				faces.clear();
			} 

			/*
			physx::PxTransform com(PxShapeExt::getGlobalPose(*shapes[j], *actors[i]));  
			physx::PxVec3   p = com.p;
			physx::PxQuat   q = com.q;
			std::cout<< itime <<" " << i <<" "<< j <<" "<< p.x <<" "<< p.y <<" "<< p.z <<"\n";
			*/
		}
	}
}

//---------------------------------------------------------------------------//
void rigidBodiesCreate() 
{	
	return; 

	bool inserted = true; 	
	PxTriangleMeshGeometry geo;  
	geo = TeddyCreate(gPhysics, inserted); 

	geo = Buda2Create(gPhysics, inserted); 

	geo = TeaPotCreate(gPhysics, inserted); 
	AddRandomGeometries(gScene, gPhysics, geo, gMaterial, 1, 10, 9, 10); 
}


//---------------------------------------------------------------------------//
void rigidBodiesEvolve(int iteration)
{
	return ; 

	PxU32 nbActors = gScene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC);
	if(nbActors)
	{
		std::vector<PxRigidActor*> actors(nbActors);
		gScene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC, reinterpret_cast<PxActor**>(&actors[0]), nbActors);
		rigidBodiesRenderActors(&actors[0], static_cast<PxU32>(actors.size()), iteration);
	}	

	physx::PxU32 nStatics = gScene->getNbActors(physx::PxActorTypeFlag::eRIGID_STATIC); 
	std::vector<physx::PxActor*> Statics(nStatics); 
	gScene->getActors(physx::PxActorTypeFlag::eRIGID_STATIC, Statics.data(), Statics.size() );

	physx::PxU32 nDynamics = gScene->getNbActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC);  
	std::vector<physx::PxActor*> Dynamics(nDynamics); 
	gScene->getActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC, Dynamics.data(), Dynamics.size() ); 

	for(int i=0; i < nDynamics; i++)
	{
		physx::PxActor* a = Dynamics[i];
		physx::PxRigidDynamic *b = static_cast<physx::PxRigidDynamic*>(a);

		physx::PxTransform com = b->getGlobalPose(); 
		physx::PxVec3   p = com.p; 
		physx::PxQuat   q = com.q;
	}	
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
PxSceneDesc sceneDescCreate() 
{
	logger->log("[sceneDescCreate] .."); 
	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());

	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);

	if (!sceneDesc.cudaContextManager)
		sceneDesc.cudaContextManager = gCudaContextManager;

	sceneDesc.flags |= PxSceneFlag::eENABLE_GPU_DYNAMICS;
	sceneDesc.flags |= PxSceneFlag::eENABLE_PCM;

	//PxU32 numCores = SnippetUtils::getNbPhysicalCores();
	//gDispatcher = PxDefaultCpuDispatcherCreate(numCores == 0 ? 0 : numCores - 1);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;

	sceneDesc.broadPhaseType = PxBroadPhaseType::eGPU;
	sceneDesc.gpuMaxNumPartitions = 8;

	sceneDesc.solverType = PxSolverType::eTGS;

	logger->log("[sceneDescCreate] !!");		
	return sceneDesc; 
}


void gSceneInit() //(PxScene* scene) 
{
	logger->log("[gSceneInit] ...");

	// static PxScene* gScene = NULL;
	gScene = gPhysics->createScene( sceneDescCreate() );

	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}

	logger->log("[gSceneInit] !!");	
}


PxRigidStatic* groundPlaneScene()
{
	PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);
	return 	groundPlane; 
}


static void initScene( 	PxTolerancesScale scale )
{
	logger->log("[initScene] ...");

	gSceneInit();

	gScene->addActor( *groundPlaneScene() ); 

	PxCookingParams params(scale);
	params.meshWeldTolerance = 0.001f;
	params.meshPreprocessParams = PxMeshPreprocessingFlags(PxMeshPreprocessingFlag::eWELD_VERTICES);
	params.buildTriangleAdjacencies = false;
	params.buildGPUData = true;

	int nHardBodies = UnityHardCreate(gScene, gPhysics, logger); 
	logger->log("[UnityHard] " + std::to_string(nHardBodies) + " hardBodies created");

//	rbkObj = new RigidBodyKinematic(gPhysics, gScene, gMaterial);
//	rbkObj->Init();

	DeformableVolumesCreate(params);

	//rigidBodiesCreate(); 

	logger->log("[initScene] !!");
} 


static void releaseScene()
{
	logger->log("[releaseScene] ...");

	PX_RELEASE(gScene);
	itime = 0; 

	logger->log("[releaseScene] !!");
}


void keyPress(unsigned char key)
{
	// See : SnippetGyroscopic.cpp 
	if(key == 'p' || key == 'P') gPause = !gPause;

	if(key == 'o' || key == 'O')
	{
		gPause = true;
		gOneFrame = true;
	}

	if(gScene)
	{
		if(key >= 1 && key <= gScenarioCount)
		{
			gScenario = key - 1;
			releaseScene();
			initScene( scale );
		}

		if(key == 'r' || key == 'R')
		{
			releaseScene();
			initScene( scale );
		}
	}
}


//---------------------------------------------------------------------------//
void LoggerCreate(bool append, bool header) 
{
	//bool append = true; 
	//logger = new Logger("F:\\z2025_1\\PhysX\\PhysX50\\physx\\SpicyTech\\output.txt", append);
	logger = new Logger("spicytech.log", append, header);
	logger->log("[LoggerCreate] ...");
}


//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
void InitPhysics(bool /*interactive*/)
{
	logger->log("[InitPhysics] ...");

	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);

	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

	// initialize cuda
	PxCudaContextManagerDesc cudaContextManagerDesc;
	gCudaContextManager = PxCreateCudaContextManager(*gFoundation, cudaContextManagerDesc, PxGetProfilerCallback());
	if (gCudaContextManager && !gCudaContextManager->contextIsValid())
	{
		PX_RELEASE(gCudaContextManager);
		printf("Failed to initialize cuda context.\n");
	}

//	PxTolerancesScale scale;
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, scale, true, gPvd);
	PxInitExtensions(*gPhysics, gPvd);
/*
	PxCookingParams params(scale);
	params.meshWeldTolerance = 0.001f;
	params.meshPreprocessParams = PxMeshPreprocessingFlags(PxMeshPreprocessingFlag::eWELD_VERTICES);
	params.buildTriangleAdjacencies = false;
	params.buildGPUData = true;
*/
/*
	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());

	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);

	if (!sceneDesc.cudaContextManager)
		sceneDesc.cudaContextManager = gCudaContextManager;

	sceneDesc.flags |= PxSceneFlag::eENABLE_GPU_DYNAMICS;
	sceneDesc.flags |= PxSceneFlag::eENABLE_PCM;

	PxU32 numCores = SnippetUtils::getNbPhysicalCores();
	gDispatcher = PxDefaultCpuDispatcherCreate(numCores == 0 ? 0 : numCores - 1);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;

	sceneDesc.broadPhaseType = PxBroadPhaseType::eGPU;
	sceneDesc.gpuMaxNumPartitions = 8;

	sceneDesc.solverType = PxSolverType::eTGS;

	gScene = gPhysics->createScene(sceneDesc);
*/
/*
	gScene = gPhysics->createScene( sceneDescCreate() );

	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
*/
/*
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.f);
	PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);
*/
	PxU32 numCores = SnippetUtils::getNbPhysicalCores();
	gDispatcher = PxDefaultCpuDispatcherCreate(numCores == 0 ? 0 : numCores - 1);

	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.f);

	initScene( scale );

	logger->log("[InitPhysics] hardBodies.size: "  + std::to_string( hardBodies.size() ));
	logger->log("[InitPhysics] gDeformableVolumes.size: "  + std::to_string( gDeformableVolumes.size() ));
	logger->log("[InitPhysics] !!");
	//return gDeformableVolumes.size(); 
}


int UnityDeformablesSizeGet() 
{
	return gDeformableVolumes.size(); 
}


//---------------------------------------------------------------------------//
void deformableVolumesEvolve(
	int iteration, 
	std::vector< std::vector<int> >& Triangles, 
	std::vector< std::vector<float> >& PositionsInvMass 
)
{
	logger->log("[deformableVolumesEvolve] ...");

	Triangles.clear(); 
	PositionsInvMass.clear(); 
	for (PxU32 ibody = 0; ibody < gDeformableVolumes.size(); ibody++)
	{		
		DeformableVolume* dv = &gDeformableVolumes[ibody];

		std::vector<float> mpositionsinvmass;
		mpositionsinvmass = dv->copyDeformedVerticesFromGPU(iteration, ibody);
		PositionsInvMass.push_back(mpositionsinvmass); 

		PxVec4* positionsInvMass = dv->mPositionsInvMass; 
		PxDeformableVolume* deformableVolume = dv->mDeformableVolume; 

		std::vector<int> triangles; 
		int nTriangles = DeformableVolumeGetConnectivity(dv->mDeformableVolume, triangles); 
		Triangles.push_back(triangles); 

		logger->log(
			"[UnityDeformableVolumes] iteration:" + std::to_string( iteration ) +
			" ibody:" + std::to_string( ibody ) +
			" nbVertices:" + std::to_string( mpositionsinvmass.size() / 4 ) + 
			" nTriangles:" + std::to_string(  triangles.size() / 3 ) 
		);		
	} //for 

	logger->log("[deformableVolumesEvolve] !!");
}


//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool stepPhysics() //(bool /*interactive*/)
{
	if (gPause && !gOneFrame) return false;
	gOneFrame = false;

	gScene->simulate(deltaTime);
	return gScene->fetchResults(true);
}

//---------------------------------------------------------------------------//
int StepPhysics(bool /*interactive*/, 
	int iteration, 
	std::vector< std::vector<int> >& Triangles, 
	std::vector< std::vector<float> >& PositionsInvMass 
)
{
	logger->log("[StepPhysics] ...");

	bool ready = stepPhysics(); 
	logger->log("[StepPhysics] ready:" + std::to_string(ready) + " --------------------------------------");

	if(ready)
	{ 
		//rigidBodiesEvolve(iteration); 
		deformableVolumesEvolve(iteration, Triangles, PositionsInvMass); 
//		rbkObj->Update(iteration, deltaTime);
		std::vector< std::vector<float> > poses; 
		UnityHardUpdate(iteration, deltaTime, logger); 
/*
		std::vector<float> position; 
		position = UnityHardGlobalPoseGet(0); 
		logger->log("[UnityHard] pose : " + std::to_string(position.size()) );

		position = UnityHardPositionGet(0);
		logger->log("[UnityHard] position:" + std::to_string(position[0]) + " "+ std::to_string(position[1]) + " "+ std::to_string(position[2]) + " ");
*/
		logger->log("[StepPhysics] itime:" + std::to_string(itime) +" Done !! --------------------------", false);
		itime++; 
	} // ready 

	logger->log("[StepPhysics] !!");
	return itime; 
} // StepPhysics


//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
void CleanupPhysics(bool /*interactive*/)
{
	logger->log("[CleanupPhysics] ...");

//	rbkObj->Finish(); 
//	delete rbkObj; 

	UnityHardClear(); 

	UnitySoftClear(); 

	for (PxU32 i = 0; i < gDeformableVolumes.size(); i++) gDeformableVolumes[i].release();
	gDeformableVolumes.reset();
	gSkinnedMeshes.reset();

	// cleanupPhysics 
	releaseScene(); //PX_RELEASE(gScene);
	PX_RELEASE(gDispatcher);
	PX_RELEASE(gPhysics);
	if (gPvd)
	{
		PxPvdTransport* transport = gPvd->getTransport();
		PX_RELEASE(gPvd);
		PX_RELEASE(transport);
	}
	PxCloseExtensions();
	PX_RELEASE(gCudaContextManager);
	PX_RELEASE(gFoundation);
	// cleanupPhysics 

	logger->log("[CleanupPhysics] !!");
	if (logger != nullptr) {
		delete logger;
		logger = nullptr;
	}
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//