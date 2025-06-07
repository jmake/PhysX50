#pragma once
#include <vector>
#include <iostream>

#include <iterator>
#include <algorithm>


std::vector<float> LoadFileFlat(const char* input, int* rows, int* cols);

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

void UnityInit(float* outArray1, int n1, int* outArray2, int n2); 

void LoggerCreate(bool); 

class SpicyX 
{
    public :
    SpicyX()
    {
        LoggerCreate( false);
        interactive = false; 
    };
/*
    int GetFlatArraySize() const;
    void InitFlatArray(int n); 
    void GetFlatArrayRaw(float* outArray1) const;
*/
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


    void MeshAdd(float* outArray1, int n1, int* outArray2, int n2)
    {
        UnityInit(outArray1, n1, outArray2, n2); 
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


    private:
        std::vector<float> flatArray; 
	    std::vector< std::vector<int> > Triangles; 
        std::vector< std::vector<float> > PositionsInvMass; 
               
        int nDeformables; 
        int nTriangles; 
        int nbVertices; 

        bool interactive; 
};


