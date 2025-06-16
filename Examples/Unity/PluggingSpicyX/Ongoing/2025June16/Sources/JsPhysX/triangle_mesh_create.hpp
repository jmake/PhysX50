#pragma once


#include <ctype.h>
#include <vector>

#include <map>
#include <string>
#include <iostream>
#include <assert.h>
#include <fstream>

#include "PxPhysicsAPI.h"

using namespace physx;

PxTriangleMeshGeometry Buda2Create(PxPhysics*, bool);  
PxTriangleMeshGeometry TeddyCreate(PxPhysics*, bool);  
PxTriangleMeshGeometry TeaPotCreate(PxPhysics*, bool); 

PxRigidDynamic*
CreateRigidDynamicWithShape2(PxPhysics *physics, const PxGeometry& g, const PxTransform& t, PxMaterial* m); 

PxRigidStatic*
CreateRigidStaticWithShape2(PxPhysics *physics, const PxGeometry& g, const PxTransform& t, PxMaterial* m); 

PxRigidDynamic* 
createDynamic(PxScene* scene, PxPhysics *physics, const PxGeometry& g, const PxTransform& t, PxMaterial* m, const PxVec3& velocity=PxVec3(0)); 


void AddRandomGeometries(PxScene* scene, PxPhysics *physics, const PxGeometry& g, PxMaterial* m, 
                        int ngeoms, float max, float ymin, float ymax);


void GetTriangleMesh(const PxGeometry& geom, 
                            std::vector< std::vector<double> >& vertices, 
                            std::vector< std::vector<int> >& faces); 


int FileLoader(std::string fname, std::vector< std::vector<float> > &array); 

