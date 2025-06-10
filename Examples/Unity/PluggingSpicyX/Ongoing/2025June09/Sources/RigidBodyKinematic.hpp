#pragma once
#include <vector>
#include <iostream>

#include <iterator>
#include <algorithm>

#include <ctype.h>
#include <fstream>
#include <iostream>
#include <cstring>    // for std::memcpy


#include "PxPhysicsAPI.h"

using namespace physx;


void _setupCommonCookingParams(PxCookingParams& params, bool skipMeshCleanup, bool skipEdgeData)
{
		// we suppress the triangle mesh remap table computation to gain some speed, as we will not need it 
	// in this snippet
	params.suppressTriangleMeshRemapTable = true;

	// If DISABLE_CLEAN_MESH is set, the mesh is not cleaned during the cooking. The input mesh must be valid. 
	// The following conditions are true for a valid triangle mesh :
	//  1. There are no duplicate vertices(within specified vertexWeldTolerance.See PxCookingParams::meshWeldTolerance)
	//  2. There are no large triangles(within specified PxTolerancesScale.)
	// It is recommended to run a separate validation check in debug/checked builds, see below.

	if (!skipMeshCleanup)
		params.meshPreprocessParams &= ~static_cast<PxMeshPreprocessingFlags>(PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH);
	else
		params.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;

	// If eDISABLE_ACTIVE_EDGES_PRECOMPUTE is set, the cooking does not compute the active (convex) edges, and instead 
	// marks all edges as active. This makes cooking faster but can slow down contact generation. This flag may change 
	// the collision behavior, as all edges of the triangle mesh will now be considered active.
	if (!skipEdgeData)
		params.meshPreprocessParams &= ~static_cast<PxMeshPreprocessingFlags>(PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE);
	else
		params.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;
}


PxTriangleMesh* 
_createBV33TriangleMesh(PxU32 numVertices, const PxVec3* vertices, PxU32 numTriangles, const PxU32* indices, 
	bool skipMeshCleanup, bool skipEdgeData, bool inserted, bool cookingPerformance, bool meshSizePerfTradeoff, 
PxPhysics *physics)
{
	PxTriangleMesh* triMesh = NULL;

	if (physics == NULL) 
	{
		std::cerr << "[createBV33TriangleMesh] 'physics == NULL' \n";
		return triMesh;
	}
	
	PxU64 startTime = SnippetUtils::getCurrentTimeCounterValue();

	PxTriangleMeshDesc meshDesc;
	meshDesc.points.count = numVertices;
	meshDesc.points.data = vertices;
	meshDesc.points.stride = sizeof(PxVec3);
	meshDesc.triangles.count = numTriangles;
	meshDesc.triangles.data = indices;
	meshDesc.triangles.stride = 3 * sizeof(PxU32);

	PxTolerancesScale scale;
	PxCookingParams params(scale);

	// Create BVH33 midphase
	params.midphaseDesc = PxMeshMidPhase::eBVH33;
	params.buildGPUData = true; // internal error : Non-GPU-compatible triangle mesh is not able to collide with deformable volume tetrahedron mesh.

	// setup common cooking params
	_setupCommonCookingParams(params, skipMeshCleanup, skipEdgeData);

	// The COOKING_PERFORMANCE flag for BVH33 midphase enables a fast cooking path at the expense of somewhat lower quality BVH construction.	
	if (cookingPerformance)
		params.midphaseDesc.mBVH33Desc.meshCookingHint = PxMeshCookingHint::eCOOKING_PERFORMANCE;
	else
		params.midphaseDesc.mBVH33Desc.meshCookingHint = PxMeshCookingHint::eSIM_PERFORMANCE;

	// If meshSizePerfTradeoff is set to true, smaller mesh cooked mesh is produced. The mesh size/performance trade-off
	// is controlled by setting the meshSizePerformanceTradeOff from 0.0f (smaller mesh) to 1.0f (larger mesh).
	if(meshSizePerfTradeoff)
	{
		params.midphaseDesc.mBVH33Desc.meshSizePerformanceTradeOff = 0.0f;
	}
	else
	{
		// using the default value
		params.midphaseDesc.mBVH33Desc.meshSizePerformanceTradeOff = 0.55f;
	}

#if defined(PX_CHECKED) || defined(PX_DEBUG)
	// If DISABLE_CLEAN_MESH is set, the mesh is not cleaned during the cooking. 
	// We should check the validity of provided triangles in debug/checked builds though.
	if (skipMeshCleanup)
	{
		PX_ASSERT(PxValidateTriangleMesh(params, meshDesc));
	}
#endif // DEBUG


//	PxTriangleMesh* triMesh = NULL;
	PxU32 meshSize = 0;

	// The cooked mesh may either be saved to a stream for later loading, or inserted directly into PxPhysics.
	if (inserted)
	{
		triMesh = PxCreateTriangleMesh(params, meshDesc, physics->getPhysicsInsertionCallback());
	}
	else
	{
		PxDefaultMemoryOutputStream outBuffer;
		PxCookTriangleMesh(params, meshDesc, outBuffer);

		PxDefaultMemoryInputData stream(outBuffer.getData(), outBuffer.getSize());
		triMesh = physics->createTriangleMesh(stream);

		meshSize = outBuffer.getSize();
	}

	// Print the elapsed time for comparison
	PxU64 stopTime = SnippetUtils::getCurrentTimeCounterValue();
	float elapsedTime = SnippetUtils::getElapsedTimeInMilliseconds(stopTime - startTime);
	printf("\t -----------------------------------------------\n");
	printf("\t Create triangle mesh with %d triangles: \n",numTriangles);
	cookingPerformance ? printf("\t\t Cooking performance on\n") : printf("\t\t Cooking performance off\n");
	inserted ? printf("\t\t Mesh inserted on\n") : printf("\t\t Mesh inserted off\n");
	!skipEdgeData ? printf("\t\t Precompute edge data on\n") : printf("\t\t Precompute edge data off\n");
	!skipMeshCleanup ? printf("\t\t Mesh cleanup on\n") : printf("\t\t Mesh cleanup off\n");
	printf("\t\t Mesh size/performance trade-off: %f \n", double(params.midphaseDesc.mBVH33Desc.meshSizePerformanceTradeOff));
	printf("\t Elapsed time in ms: %f \n", double(elapsedTime));
	if(!inserted)
	{
		printf("\t Mesh size: %d \n", meshSize);
	}

	//triMesh->release();
	//triMesh->release();
	return triMesh; 
}


PxTriangleMeshGeometry
createBunnyMesh(
    //std::string fcoords, std::string ftrias, 
	std::vector< std::vector<float> > coords, 
	std::vector< std::vector<int> > trias, 
    PxPhysics *physics, bool inserted) //, PxCooking *cooking)
{
/*
	std::vector< std::vector<float> > coords;
	LoadFile(fcoords, coords, false);

	std::vector< std::vector<int> > trias;
	LoadFile(ftrias, trias, false);
*/
// createTriangleMeshes
// PxVec3* vertices = new PxVec3[numVertices];
// PxU32* indices = new PxU32[numTriangles * 3];

	PxU32 numVertices = 0; // verts.size(); 
	PxU32 numTriangles = 0; // faces.size() / 3; 

	std::vector<PxVec3> verts;
	for(auto c:coords) 
	{
		if(c.size() == 3) 
		{
			verts.push_back( PxVec3(c[0],c[1],c[2]) ); 
			numVertices++; 
		}
	}

	std::vector<PxU32> faces;
	for(auto tria:trias) 
	{ 
		if(tria.size() == 3) 
		{
			for(auto t:tria) faces.push_back(t);
			numTriangles++;
		};
	} 

	PxTriangleMesh* mesh = NULL; 
	if(coords.size() > 0 && faces.size() ) 
	{
		mesh = _createBV33TriangleMesh(
								numVertices, verts.data(), 
								numTriangles, faces.data(), 
								false, false, inserted, false, false, 
								physics);  

	}

	PxTriangleMeshGeometry geom(mesh);
	return geom; 
}


PxRigidDynamic* CreateCube(
    PxScene* scene, 
    PxPhysics *physics)
{
    std::vector<std::vector<float>> vertices = {
        { 0.5f, -0.5f, -0.5f },
        { 0.5f, -0.5f,  0.5f },
        {-0.5f, -0.5f,  0.5f },
        {-0.5f, -0.5f, -0.5f },
        { 0.5f,  0.5f, -0.5f },
        { 0.5f,  0.5f,  0.5f },
        {-0.5f,  0.5f,  0.5f },
        {-0.5f,  0.5f, -0.5f }
    };

    std::vector<std::vector<int>> triangles = {
        {1, 2, 3},
        {7, 6, 5},
        {4, 5, 1},
        {5, 6, 2},
        {2, 6, 7},
        {0, 3, 7},
        {0, 1, 3},
        {4, 7, 5},
        {0, 4, 1},
        {1, 5, 2},
        {3, 2, 7},
        {4, 0, 7}
    };

    bool inserted = false; 

    PxTriangleMeshGeometry mesh; 
    mesh = createBunnyMesh(vertices, triangles, physics, inserted); 

    PxVec3 p(0.0,0.0,1.0);
    PxQuat q;
PxQuat YZ(0.0,0.0,1.0,1.0);
PxQuat XZ(0.0,1.0,0.0,1.0);
PxQuat XY(1.0,0.0,0.0,1.0);
	PxTransform t = PxTransform(p,XZ);  
//MIRAR AQUI!!

    PxMaterial* material = physics->createMaterial(0.5f, 0.5f, 0.0f);

	PxRigidDynamic* actor = physics->createRigidDynamic(t);
    PxRigidActorExt::createExclusiveShape(*actor, mesh, *material);

    actor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
    scene->addActor(*actor);
    return actor; 
/*
	PxScene* scene; 
	PxPhysics *physics; 
    const PxGeometry& g;  
    PxMaterial* m; 

    PxVec3 p(xr,yr,zr);
    PxQuat q(qx,qy,qz,qw);

    //PxRigidDynamic* actor = CreateRigidDynamicWithShape2(physics, g, PxTransform(p,q), m);

    PxTransform t = PxTransform(p,q);  
	PxRigidDynamic* actor = physics->createRigidDynamic(t);
	//PxShape* shape = PxRigidActorExt::createExclusiveShape(*actor, g, *m); //?? 

    scene->addActor(*actor);
*/
}


class RigidBodyKinematic
{
public:
    RigidBodyKinematic(PxPhysics* physics, PxScene* scene, PxMaterial* material) : 
        gPhysics(physics), 
        gScene(scene), 
        gMaterial(material), 
        rigidBody(nullptr), 
        simTime(0.0f)
    {
		rigidBody = nullptr;
	}


    void Init()
    {
		rigidBody = nullptr;
		//rigidBody = SphereTest(); 
        rigidBody = CreateCube(gScene, gPhysics); 
    }


     PxRigidDynamic* SphereTest() 
    {
        PxSphereGeometry geo = PxSphereGeometry(3.0f);  

        PxShape* shape = gPhysics->createShape(geo, *gMaterial);

        PxRigidDynamic* rb = gPhysics->createRigidDynamic(PxTransform(PxVec3(0.f, 5.0f, 0.f)));
        rb->attachShape(*shape);
        rb->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
        gScene->addActor(*rb);
        shape->release();

        PositionSet(0.0, 0.0, 0.0); 
        return rb; 
    }


    void PositionSet(float x, float y, float z)
    {
        newPose = PxTransform(PxVec3(x, y, z));
    }


    void PositionGet(float& x, float& y, float& z)
    {
        if (rigidBody)
        {
			physx::PxTransform com = rigidBody->getGlobalPose(); 
			physx::PxVec3   p = com.p; 
			x=p.x; y=p.y; z=p.z;
			//x=-1; y=-2; z=-3; 
		}
		else 
		{
			x=1; y=2; z=3; 
		} 
    }


    void Update(int iteration, PxReal deltaTime)
    {
        simTime += deltaTime;
        const PxReal speed = 2.0f;

		physx::PxTransform com = rigidBody->getGlobalPose(); 
		physx::PxVec3   p = com.p; 
		physx::PxQuat   q = com.q; 
		printf("[RigidBodyKinematic] %d) pose:(%f, %f, %f) \n", iteration, p.x, p.y, p.z);

        //PxTransform pose = rigidBody->getGlobalPose();        
		//printf("[KinematicrigidBody] %d) pose:(%f, %f, %f) \n", iteration, pose.p.x, pose.p.y, pose.p.z);
        //PxQuat rotation = PxQuat(PxCos(simTime * speed), PxVec3(0, 1, 0));
        //rigidBody->setKinematicTarget(PxTransform(pose.p, rotation));

        rigidBody->setKinematicTarget(newPose);
    }


    void Finish()
    {
        if (rigidBody)
        {
            rigidBody->release();
            rigidBody = nullptr;
        }
    }


    PxRigidDynamic* getActor() const { return rigidBody; }

private:
    PxScene* gScene;
    PxPhysics* gPhysics;
    PxMaterial* gMaterial;
    PxRigidDynamic* rigidBody;

    PxReal simTime;
    PxTransform newPose; 
};
