#pragma once
#include <vector>
#include <iostream>

#include <iterator>
#include <algorithm>


float PhysxVersion(); 
int CudaVersion(); 

int SpicyTest(); 


class SpicyX 
{
    public:
    SpicyX(int x);
    int GetValue() const;
    void SetValue(int x);

    std::vector<float>& GetFlatArray(); 

void GetFlatArrayRaw(float* outArray) const;
int GetFlatArraySize() const;
void InitFlatArray(int n); 

    private:
    int value;
    std::vector<float> flatArray; 
};