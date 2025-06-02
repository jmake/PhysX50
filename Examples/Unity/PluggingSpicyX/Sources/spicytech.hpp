#pragma once
#include <vector>
#include <iostream>

#include <iterator>
#include <algorithm>


int CudaVersion(); 
int SpicyTest(); 
float PhysxVersion(); 

int RenderInit(); 
void RenderFinish(); 

int StepPhysics(bool /*interactive*/, 
                    int iteration, 
	                std::vector< std::vector<int> >& Triangles, 
                    std::vector< std::vector<float> >& PositionsInvMass
                ); 

class SpicyX 
{
    public:
    SpicyX();

    int GetFlatArraySize() const;
    void InitFlatArray(int n); 
    void GetFlatArrayRaw(float* outArray1) const;

    void Finish()
    {
        RenderFinish(); 
    }

    int Init() 
    {
        nDeformables = RenderInit(); 
		std::cout<<"[SpicyX] nDeformables:" << nDeformables << std::endl;
        return nDeformables; 
    }


    int Step(int i)
    {
        int itime = StepPhysics(false, i, Triangles, PositionsInvMass);
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

    
    private:
        std::vector<float> flatArray; 
	    std::vector< std::vector<int> > Triangles; 
        std::vector< std::vector<float> > PositionsInvMass; 
               
        int nDeformables; 
        int nTriangles; 
        int nbVertices; 
};
