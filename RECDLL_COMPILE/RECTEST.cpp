#include <Windows.h>
#include <stdio.h>
typedef void* (*LoadedFunc)(const char* PATH ,unsigned char res_multiplier, float samplingD, int* VertexCount, double* timing);
int main(){
    auto DLL = LoadLibrary("RecDLL.dll");
    LoadedFunc f = (LoadedFunc)GetProcAddress(DLL, "LoadPC_andReconstruct");
    int VertexCount = 0; double timing = 0.0;
    f("C:/Users/furka/Desktop/Meshes/HumanBase.off", 6, 1.12f, &VertexCount, &timing);
    printf("Finished, vertex count: %u and Timing: %f", VertexCount, timing);
}