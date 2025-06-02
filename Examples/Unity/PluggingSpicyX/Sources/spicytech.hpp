#pragma once
#include <vector>
#include <iostream>

#include <iterator>
#include <algorithm>


float PhysxVersion(); 
int CudaVersion(); 

int SpicyTest(); 


class SpicyX {
    public:
        SpicyX(int x);
        int GetValue() const;
        void SetValue(int x);
    private:
        int value;
};