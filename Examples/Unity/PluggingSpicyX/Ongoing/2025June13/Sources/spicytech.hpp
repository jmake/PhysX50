#pragma once
#include <vector>
#include <iostream>

#include <iterator>
#include <algorithm>


std::vector<float> LoadFileFlat(const char* input, int* rows, int* cols);
void LoggerCreate(bool, bool); 

int CudaVersion(); 
float PhysxVersion(); 

void InitPhysics(bool); 
void CleanupPhysics(bool); 

int StepPhysics(
                    bool, 
                    int iteration, 
	                std::vector< std::vector<int> >& Triangles, 
                    std::vector< std::vector<float> >& PositionsInvMass
                ); 

void keyPress(unsigned char key); 

void UnitySoftAdd(float* outArray1, int n1, int* outArray2, int n2); 


void UnityHardAdd(float* outArray1, int n1, int* outArray2, int n2, bool on);  

void UnityHardPositionSet(int i, float x, float y, float z); 
void UnityHardPositionGet(int i, float& x, float& y, float& z); 

std::vector<float> UnityHardGlobalPoseGet(int ibody);  

void UnityHardGlobalPoseInitial(float px, float py, float pz, float qx, float qy, float qz, float qw);  
void UnityHardGlobalPoseSet(int ibody, float px, float py, float pz, float qx, float qy, float qz, float qw); 


int UnityHardsSizeGet(); 
int UnityDeformablesSizeGet();  


class SpicyX 
{
public :
    SpicyX(bool append=false, bool header=false)
    {
        LoggerCreate(append, header);
        interactive = false; 
        nRigidBodies = 0; 
    };


    void Finish()
    {
        CleanupPhysics(interactive);
    }


    void Init() 
    {
        //nDeformables = 
        InitPhysics(interactive); 
		//std::cout<<"[SpicyX] nDeformables:" << nDeformables << std::endl;
        //return nDeformables; 
    }


    int SoftSize()
    {
        return UnityDeformablesSizeGet();
    }


    int HardSize()
    {
        return UnityHardsSizeGet();
    }


    void SoftAdd(float* outArray1, int n1, int* outArray2, int n2, bool on)
    {
        UnitySoftAdd(outArray1, n1, outArray2, n2); 
        nDeformables++; 
    }


    int Step(int i)
    {
        int itime = StepPhysics(interactive, i, Triangles, PositionsInvMass);
		std::cout<<"[SpicyX]"
        <<" nTriangles:" << Triangles.size()
		<<" nPositionsInvMass:" << PositionsInvMass.size()
        << std::endl;

        return itime; 
    }


    int nBodies()
    {
        return PositionsInvMass.size(); 
    }


    int PositionsInvMassSize(int i)
    {
        return PositionsInvMass[i].size(); 
    }


    void PositionsInvMassGet(int i, float* outArray1)
    {
        std::copy(PositionsInvMass[i].begin(), PositionsInvMass[i].end(), outArray1);
    }    


    int TrianglesSize(int i)
    {
        return Triangles[i].size(); 
    }


    void TrianglesGet(int i, int* outArray2)
    {
        std::copy(Triangles[i].begin(), Triangles[i].end(), outArray2);
    }    


    void KeyPress(int key) 
    {
        keyPress(static_cast<unsigned char>(key));
    }


    void HardAdd(float* outArray1, int n1, int* outArray2, int n2, bool on)
    {
        UnityHardAdd(outArray1, n1, outArray2, n2, on); 
        nRigidBodies++; 
    }


    void PositionSet(int ibody, float x, float y, float z)
    {
        if( (ibody < 0) || (ibody > nRigidBodies) ) return; 

        UnityHardPositionSet(ibody, x, y, z); 
    }


    void PositionGet(int ibody, float* outArray1)
    {
        if( (ibody < 0) || (ibody > nRigidBodies) ) return; 

        UnityHardPositionGet(ibody, outArray1[0], outArray1[1], outArray1[2]); 
    }


    void GlobalPoseGet(int ibody, float* outArray1, float* outArray3)
    {
        if( (ibody < 0) || (ibody > nRigidBodies) ) return; 

        std::vector<float> array = UnityHardGlobalPoseGet(ibody); 
        if(array.size() != 7) return; 

        std::copy(array.begin(), array.begin() + 3, outArray1);
        std::copy(array.begin()+3, array.end(), outArray3);
    }


    void GlobalPoseSet(int ibody, float px, float py, float pz, float qx, float qy, float qz, float qw)
    {
        if( (ibody < 0) || (ibody > nRigidBodies) ) return; 

        UnityHardGlobalPoseSet(ibody,
            px, py, pz, 
            qx, qy, qz, qw 
        );
    }


    void GlobalPoseInitial(float px, float py, float pz, float qx, float qy, float qz, float qw)
    {
        UnityHardGlobalPoseInitial(
            px, py, pz, 
            qx, qy, qz, qw 
        ); 
    }


    private:
        std::vector<float> flatArray; 
	    std::vector< std::vector<int> > Triangles; 
        std::vector< std::vector<float> > PositionsInvMass; 

        bool interactive; 

        int nDeformables; 
        int nTriangles; 
        int nbVertices; 


        int nRigidBodies; 
};


