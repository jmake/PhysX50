#pragma once
#include <vector>
#include <iostream>

#include <iterator>
#include <algorithm>


std::vector<float> LoadFileFlat(const char* input, int* rows, int* cols);
void LoggerCreate(bool); 

int CudaVersion(); 
float PhysxVersion(); 

int InitPhysics(bool); 
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


class SpicyX 
{
public :
    SpicyX(bool append=false)
    {
        LoggerCreate( append );
        interactive = false; 
        nRigidBodies = 0; 
    };


    void Finish()
    {
        CleanupPhysics(interactive);
    }


    int Init() 
    {
        nDeformables = InitPhysics(interactive); 
		std::cout<<"[SpicyX] nDeformables:" << nDeformables << std::endl;
        return nDeformables; 
    }


    void SoftAdd(float* outArray1, int n1, int* outArray2, int n2, bool on)
    {
        UnitySoftAdd(outArray1, n1, outArray2, n2); 
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


    private:
        std::vector<float> flatArray; 
	    std::vector< std::vector<int> > Triangles; 
        std::vector< std::vector<float> > PositionsInvMass; 
               
        int nDeformables; 
        int nTriangles; 
        int nbVertices; 

        bool interactive; 

        int nRigidBodies; 
};


