#include <ctype.h>
#include <vector>

#include <map>
#include <array>
#include <string>
#include <iostream>
#include <assert.h>
#include <fstream>



#include "PxPhysicsAPI.h"
#include "../snippetutils/SnippetUtils.h"

#include "JsPhysX/load_file.hpp" // LoadFile
#include "JsPhysX/triangle_mesh_create.hpp"


using namespace physx;

static PxDefaultAllocator		gAllocator;
static PxDefaultErrorCallback	gErrorCallback;
static PxFoundation*			gFoundation = NULL;
static PxPhysics*				gPhysics	= NULL;

float rand(float loVal, float hiVal)
{
	return loVal + float(rand()/float(RAND_MAX))*(hiVal - loVal);
}

PxU32 rand(PxU32 loVal, PxU32 hiVal)
{
	return loVal + PxU32(rand()%(hiVal - loVal));
}

void indexToCoord(PxU32& x, PxU32& z, PxU32 index, PxU32 w)
{
	x = index % w;
	z = index / w;
}

// Creates a random terrain data.
void createRandomTerrain(const PxVec3& origin, PxU32 numRows, PxU32 numColumns,
	PxReal cellSizeRow, PxReal cellSizeCol, PxReal heightScale,
	PxVec3*& vertices, PxU32*& indices)
{
	PxU32 numX = (numColumns + 1);
	PxU32 numZ = (numRows + 1);
	PxU32 numVertices = numX*numZ;
	PxU32 numTriangles = numRows*numColumns * 2;

	if (vertices == NULL)
		vertices = new PxVec3[numVertices];
	if (indices == NULL)
		indices = new PxU32[numTriangles * 3];

	PxU32 currentIdx = 0;
	for (PxU32 i = 0; i <= numRows; i++)
	{
		for (PxU32 j = 0; j <= numColumns; j++)
		{
			PxVec3 v(origin.x + PxReal(j)*cellSizeRow, origin.y, origin.z + PxReal(i)*cellSizeCol);
			vertices[currentIdx++] = v;
		}
	}

	currentIdx = 0;
	for (PxU32 i = 0; i < numRows; i++)
	{
		for (PxU32 j = 0; j < numColumns; j++)
		{
			PxU32 base = (numColumns + 1)*i + j;
			indices[currentIdx++] = base + 1;
			indices[currentIdx++] = base;
			indices[currentIdx++] = base + numColumns + 1;
			indices[currentIdx++] = base + numColumns + 2;
			indices[currentIdx++] = base + 1;
			indices[currentIdx++] = base + numColumns + 1;
		}
	}

	for (PxU32 i = 0; i < numVertices; i++)
	{
		PxVec3& v = vertices[i];
		v.y += heightScale * rand(-1.0f, 1.0f);
	}
}

// Setup common cooking params
void setupCommonCookingParams(PxCookingParams& params, bool skipMeshCleanup, bool skipEdgeData)
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

// Creates a triangle mesh using BVH33 midphase with different settings.
PxTriangleMesh* 
createBV33TriangleMesh(PxU32 numVertices, const PxVec3* vertices, PxU32 numTriangles, const PxU32* indices, 
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

	// setup common cooking params
	setupCommonCookingParams(params, skipMeshCleanup, skipEdgeData);

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

// Creates a triangle mesh using BVH34 midphase with different settings.
void createBV34TriangleMesh(PxU32 numVertices, const PxVec3* vertices, PxU32 numTriangles, const PxU32* indices,
	bool skipMeshCleanup, bool skipEdgeData, bool inserted, const PxU32 numTrisPerLeaf)
{
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

	// Create BVH34 midphase
	params.midphaseDesc = PxMeshMidPhase::eBVH34;

	// setup common cooking params
	setupCommonCookingParams(params, skipMeshCleanup, skipEdgeData);

	// Cooking mesh with less triangles per leaf produces larger meshes with better runtime performance
	// and worse cooking performance. Cooking time is better when more triangles per leaf are used.
	params.midphaseDesc.mBVH34Desc.numPrimsPerLeaf = numTrisPerLeaf;

#if defined(PX_CHECKED) || defined(PX_DEBUG)
	// If DISABLE_CLEAN_MESH is set, the mesh is not cleaned during the cooking. 
	// We should check the validity of provided triangles in debug/checked builds though.
	if (skipMeshCleanup)
	{
		PX_ASSERT(PxValidateTriangleMesh(params, meshDesc));
	}
#endif // DEBUG


	PxTriangleMesh* triMesh = NULL;
	PxU32 meshSize = 0;

	// The cooked mesh may either be saved to a stream for later loading, or inserted directly into PxPhysics.
	if (inserted)
	{
		triMesh = PxCreateTriangleMesh(params, meshDesc, gPhysics->getPhysicsInsertionCallback());
	}
	else
	{
		PxDefaultMemoryOutputStream outBuffer;
		PxCookTriangleMesh(params, meshDesc, outBuffer);

		PxDefaultMemoryInputData stream(outBuffer.getData(), outBuffer.getSize());
		triMesh = gPhysics->createTriangleMesh(stream);

		meshSize = outBuffer.getSize();
	}

	// Print the elapsed time for comparison
	PxU64 stopTime = SnippetUtils::getCurrentTimeCounterValue();
	float elapsedTime = SnippetUtils::getElapsedTimeInMilliseconds(stopTime - startTime);
	printf("\t -----------------------------------------------\n");
	printf("\t Create triangle mesh with %d triangles: \n", numTriangles);
	inserted ? printf("\t\t Mesh inserted on\n") : printf("\t\t Mesh inserted off\n");
	!skipEdgeData ? printf("\t\t Precompute edge data on\n") : printf("\t\t Precompute edge data off\n");
	!skipMeshCleanup ? printf("\t\t Mesh cleanup on\n") : printf("\t\t Mesh cleanup off\n");
	printf("\t\t Num triangles per leaf: %d \n", numTrisPerLeaf);
	printf("\t Elapsed time in ms: %f \n", double(elapsedTime));
	if (!inserted)
	{
		printf("\t Mesh size: %d \n", meshSize);
	}

	triMesh->release();
}

void createTriangleMeshes()
{	
	const PxU32 numColumns = 128;
	const PxU32 numRows = 128;
	const PxU32 numVertices = (numColumns + 1)*(numRows + 1);
	const PxU32 numTriangles = numColumns*numRows * 2;

	PxVec3* vertices = new PxVec3[numVertices];
	PxU32* indices = new PxU32[numTriangles * 3];

	srand(50);

	createRandomTerrain(PxVec3(0.0f, 0.0f, 0.0f), numRows, numColumns, 1.0f, 1.0f, 1.f, vertices, indices);

	// Create triangle mesh using BVH33 midphase with different settings
	printf("-----------------------------------------------\n");
	printf("Create triangles mesh using BVH33 midphase: \n\n");

	// Favor runtime speed, cleaning the mesh and precomputing active edges. Store the mesh in a stream.
	// These are the default settings, suitable for offline cooking.
	createBV33TriangleMesh(numVertices,vertices,numTriangles,indices, false, false, false, false, false, gPhysics);

	// Favor mesh size, cleaning the mesh and precomputing active edges. Store the mesh in a stream.
	createBV33TriangleMesh(numVertices, vertices, numTriangles, indices, false, false, false, false, true, gPhysics);

	// Favor cooking speed, skip mesh cleanup, but precompute active edges. Insert into PxPhysics.
	// These settings are suitable for runtime cooking, although selecting fast cooking may reduce
	// runtime performance of simulation and queries. We still need to ensure the triangles 
	// are valid, so we perform a validation check in debug/checked builds.
	createBV33TriangleMesh(numVertices,vertices,numTriangles,indices, true, false, true, true, false, gPhysics);

	// Favor cooking speed, skip mesh cleanup, and don't precompute the active edges. Insert into PxPhysics.
	// This is the fastest possible solution for runtime cooking, but all edges are marked as active, which can
	// further reduce runtime performance, and also affect behavior.
	createBV33TriangleMesh(numVertices,vertices,numTriangles,indices, false, true, true, true, false, gPhysics);

	// Create triangle mesh using BVH34 midphase with different settings
	printf("-----------------------------------------------\n");
	printf("Create triangles mesh using BVH34 midphase: \n\n");

	// Favor runtime speed, cleaning the mesh and precomputing active edges. Store the mesh in a stream.
	// These are the default settings, suitable for offline cooking.
	createBV34TriangleMesh(numVertices, vertices, numTriangles, indices, false, false, false, 4);

	// Favor mesh size, cleaning the mesh and precomputing active edges. Store the mesh in a stream.
	createBV34TriangleMesh(numVertices, vertices, numTriangles, indices, false, false, false, 15);

	// Favor cooking speed, skip mesh cleanup, but precompute active edges. Insert into PxPhysics.
	// These settings are suitable for runtime cooking, although selecting more triangles per leaf may reduce
	// runtime performance of simulation and queries. We still need to ensure the triangles 
	// are valid, so we perform a validation check in debug/checked builds.
	createBV34TriangleMesh(numVertices, vertices, numTriangles, indices, true, false, true, 15);

	// Favor cooking speed, skip mesh cleanup, and don't precompute the active edges. Insert into PxPhysics.
	// This is the fastest possible solution for runtime cooking, but all edges are marked as active, which can
	// further reduce runtime performance, and also affect behavior.
	createBV34TriangleMesh(numVertices, vertices, numTriangles, indices, false, true, true, 15);

	delete [] vertices;
	delete [] indices;
}

void initPhysics()
{
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(),true);
	
	createTriangleMeshes();
}
	
void cleanupPhysics()
{
	PX_RELEASE(gPhysics);
	PX_RELEASE(gFoundation);
	
	printf("SnippetTriangleMeshCreate done.\n");
}

/*
int snippetMain(int, const char*const*)
{	
	initPhysics();
	cleanupPhysics();

	return 0;
}
*/

//-----------------------------------------------------------------//
//-----------------------------------------------------------------//
PxTriangleMeshGeometry
createBunnyMesh(std::string fcoords, std::string ftrias, PxPhysics *physics, bool inserted) //, PxCooking *cooking)
{
	std::vector< std::vector<float> > coords;
	LoadFile(fcoords, coords, false);

	std::vector< std::vector<int> > trias;
	LoadFile(ftrias, trias, false);

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
		mesh = createBV33TriangleMesh(
								numVertices, verts.data(), 
								numTriangles, faces.data(), 
								false, false, inserted, false, false, 
								physics);  

	}

	PxTriangleMeshGeometry geom(mesh);
	return geom; 
}


PxTriangleMeshGeometry 
TeddyCreate(PxPhysics *physics, bool inserted) 
{
	std::string fcoords;
	std::string ftrias; 

	fcoords = "Scripts/teddy_verts.dat";
	ftrias =  "Scripts/teddy_faces.dat"; 
	return createBunnyMesh(fcoords, ftrias, physics, inserted);	
}


PxTriangleMeshGeometry 
TeaPotCreate(PxPhysics *physics, bool inserted) 
{
	std::string fcoords;
	std::string ftrias; 

	fcoords = "Scripts/teapot_verts.dat";
	ftrias =  "Scripts/teapot_faces.dat"; 
	return createBunnyMesh(fcoords, ftrias, physics, inserted);	
}

PxTriangleMeshGeometry 
Buda2Create(PxPhysics *physics, bool inserted) 
{
	std::string fcoords;
	std::string ftrias; 

	fcoords = "Scripts/buda2_verts.dat";
	ftrias =  "Scripts/buda2_faces.dat"; 
	return createBunnyMesh(fcoords, ftrias, physics, inserted);	
}


PxRigidDynamic*
CreateRigidDynamicWithShape2(PxPhysics *physics, const PxGeometry& g, const PxTransform& t, PxMaterial* m)
{
	//PxMaterial*  m = gMaterial;

	PxRigidDynamic* actor = physics->createRigidDynamic(t);
	PxShape* shape = PxRigidActorExt::createExclusiveShape(*actor, g, *m);
	return actor;
}


PxRigidStatic*
CreateRigidStaticWithShape2(PxPhysics *physics, const PxGeometry& g, const PxTransform& t, PxMaterial* m)
{ 
	//PxMaterial*  m = gMaterial;
	PxRigidStatic* actor = physics->createRigidStatic(t);
	PxShape* shape = PxRigidActorExt::createExclusiveShape(*actor, g, *m);
	return actor;
}


PxRigidDynamic* 
createDynamic(PxScene* scene, PxPhysics *physics, const PxGeometry& g, const PxTransform& t, PxMaterial* m, const PxVec3& velocity)
{
	PxRigidDynamic* dynamic = PxCreateDynamic(*physics, t, g, *m, 10.0f);
	dynamic->setAngularDamping(0.5f);
	dynamic->setLinearVelocity(velocity);

	scene->addActor(*dynamic);
	return dynamic;
}


float Rand(float loVal, float hiVal)
{
  return loVal + (float(rand())/float(RAND_MAX))*(hiVal - loVal);
}


void AddRandomGeometries(
	PxScene* scene,
	PxPhysics *physics, const PxGeometry& g, PxMaterial* m, 
	int ngeoms, float max, float ymin, float ymax)
{
  for(int i=0; i<ngeoms; i++)
  { 
    float xr = Rand(-0.9*max,0.9*max);
    float zr = Rand(-0.9*max,0.9*max);
    float yr = Rand(ymin, ymax);
    physx::PxVec3 p(xr,yr,zr);
    
    float qx = Rand(-1.0,1.0);
    float qy = Rand(-1.0,1.0);
    float qz = Rand(-1.0,1.0);
    float qw = Rand(-3.1415,3.1415);
    physx::PxQuat q(qx,qy,qz,qw);
    
    physx::PxRigidDynamic* actor = CreateRigidDynamicWithShape2(physics, g, physx::PxTransform(p,q), m);
    scene->addActor(*actor);
  }
} 


void GetTriangleMesh(const PxGeometry& geom, 
                            std::vector< std::vector<double> >& vertices, 
                            std::vector< std::vector<int> >& faces)
{
  if(geom.getType() == PxGeometryType::eTRIANGLEMESH)
  {
      const PxTriangleMeshGeometry& triGeom = static_cast<const PxTriangleMeshGeometry&>(geom);
      const PxVec3 scale = triGeom.scale.scale;

      const PxTriangleMesh& mesh = *triGeom.triangleMesh;

      const PxU32 nbVerts       = mesh.getNbVertices();
      const PxVec3* vertexBuffer = mesh.getVertices();
      for(int i=0; i<nbVerts; i++)
      {
        PxVec3 p = vertexBuffer[i];
        vertices.push_back( {p.x,p.y,p.z} );
      }       

      const void* indexBuffer = mesh.getTriangles();
      const PxU32* intIndices = reinterpret_cast<const PxU32*>(indexBuffer);
      const PxU16* shortIndices = reinterpret_cast<const PxU16*>(indexBuffer);

      const PxU32 triangleCount = mesh.getNbTriangles();
      const PxU32 has16BitIndices = mesh.getTriangleMeshFlags() & PxTriangleMeshFlag::e16_BIT_INDICES;
      for(PxU32 i=0; i < triangleCount; ++i)
      {
        PxVec3 triVert[3];

        if(has16BitIndices)
        {
          int id0 = *shortIndices++; 
          int id1 = *shortIndices++; 
          int id2 = *shortIndices++; 

          triVert[0] = vertexBuffer[id0];
          triVert[1] = vertexBuffer[id1];
          triVert[2] = vertexBuffer[id2];
faces.push_back( {id0,id1,id2} );
        }
        else
        {
          int id0 = *shortIndices++; 
          int id1 = *shortIndices++; 
          int id2 = *shortIndices++; 

          triVert[0] = vertexBuffer[id0];
          triVert[1] = vertexBuffer[id1];
          triVert[2] = vertexBuffer[id2];
faces.push_back({id0, id1, id2});
        }
      }       
  }
}

//-----------------------------------------------------------------//
//-----------------------------------------------------------------//