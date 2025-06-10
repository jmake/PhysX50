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


void UnityHardAdd(float* outArray1, int n1, int* outArray2, int n2);  
void UnityHardPositionSet(float x, float y, float z); 
void UnityHardPositionGet(float& x, float& y, float& z); 


class SpicyX 
{
public :
    SpicyX(bool append=false)
    {
        LoggerCreate( append );
        interactive = false; 
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


    void SoftAdd(float* outArray1, int n1, int* outArray2, int n2)
    {
        UnitySoftAdd(outArray1, n1, outArray2, n2); 
    }


    void HardAdd(float* outArray1, int n1, int* outArray2, int n2)
    {
        UnityHardAdd(outArray1, n1, outArray2, n2); 
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


    void PositionSet(float x, float y, float z)
    {
        UnityHardPositionSet(x, y, z); 
    }


    void PositionGet(float* outArray1)
    {
        UnityHardPositionGet(outArray1[0], outArray1[1], outArray1[2]); 
    }


    private:
        std::vector<float> flatArray; 
	    std::vector< std::vector<int> > Triangles; 
        std::vector< std::vector<float> > PositionsInvMass; 
               
        int nDeformables; 
        int nTriangles; 
        int nbVertices; 

        bool interactive; 
};


