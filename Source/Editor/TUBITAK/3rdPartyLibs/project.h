
#ifndef PROJECT_H_
#define PROJECT_H_

#include <iostream>
#include <string>
#include <fstream>
#include <ostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <cmath>
#include <queue>


//Constants
#define MAXRHO 92
#define RADIUS 4
#define TILEWIDTH (2*RADIUS+1)
#define BLOCKWIDTH 17



//Point in 3-dimensional space
struct point{
    double x;
    double y;
    double z;
};

//Struct for comparing elements of winner array
struct winnerElement{
    unsigned long long votes;
    int index;
    bool operator<(const winnerElement& other) const
    {
        if(other.votes>votes){
            return true;
        }
        else if(other.votes==votes&&index<other.index){
            return true;
        }
        else{
            return false;
        }
    }
};

//Plane in spherical coordinate system
struct plane{
    int rho;
    int theta;
    int phi;
};

//Color in RGB
struct color{
    int r;
    int g;
    int b;
};

//Could have used same struct for point,plane, and color but did this to avoid confusion.

//Function prototypes
const point c_points[2047];
unsigned long long* CPUHough(point points[],long pointCount,int rad, int WinnerCount);
void printPlane(int i);
plane makePlane(int i);

#endif
