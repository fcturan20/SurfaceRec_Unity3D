## Geometry Processing
This project is to provide surface reconstruction from 3D point cloud to 3D triangle mesh. It was using TuranEngine before, but now is ported to Unity.
Reconstruction algorithm is all runs in "RecDLL.dll" and there is a C# script to get the reconstruction data out from the DLL.
I don't know how Unity imports and manages point clouds, so plugin uses a path to an file format that Assimp library supports and loads the object as point cloud. Then reconstruct according to the samplingD and Resolution Multiplier parameters.
Algorithm is so simple: Signed Distance Field sampling is used on point cloud, then marching cubes is used to extract triangle mesh from the SDF. Using SDF sampling on a point cloud results in lots of ambiguous cases and there a lot of research on this subject but this is implemented for a very specific project that;
1) Needs real-time reconstruction on all devices 
2) Point clouds assumed to be very dense
3) Artifacts can be minimized by applying temporal filtering to results of multiple frames
4) Surface normals positivity and Surface color/texture can be guessed
So final algorithm is multi-threaded (can even be moved to GPGPU safely) and reconstruction is only dependent to count of new/position changing points.

## Usage
SamplingD: Defines how far a point can be to a SDF sample.
Resolution Multiplier: Defines SDF sample count. Value is multiplied by 10. SDF samples are distributed uniformly across the bounding box of the point cloud. Don't forget to increase SamplingD value after increasing this value.

## RecDLL Source Code Detail
1) Threading system of the TuranLibraries is initialized
2) Point cloud is imported with Assimp from a vertex mesh or something
3) Bounding box of the point cloud is computed
4) DLLs variables are initialized according to inputs (Unity imports the DLL only ones, so arrays are cleared too)
5) SDF sampling is started across all threads:
    A)  For each point in the cloud, calculate nearest N + 1 SDF samples according to the samplingD value.
    B)  For each possible sample; 
        a)  Calculate distance between the sample and the point
        b)  Lock Sample data, check the distance with the distance value that Sample stores
        c)  If point is nearer then the point stored in Sample, change nearest value in Sample data.
        d)  Free lock.
6) Resulting SDF values are used to extract mesh with Marching Cubes across all threads.
7) Resulting mesh datas across all jobs are gathered in a single, position only vertex buffer.
8) Threading system of the TuranLibraries is closed.
Note: Algorithm is designed to allow incremental changes in the point cloud, so there are only minor changes in the functionality to support them.

## Previous Implementation
SDF sampling was searching in the reverse direction. I mean the approach was, instead of (point->nearest sample), (sample-> nearest points). To find nearest points for a SDF sample, I used a KD-Tree library to generate kdtree from point cloud and query nearest N points for each SDF sample. But there were lots of limitations:
1) Even though kdtree generation was fast for a 12k point cloud (5ms), it'd be way too slower for the project's target point clouds (hundreds of thousands of points). 
2) Queries for nearest samples were taking around 1-2 seconds at best (with Multiplier: 6, SamplingD: 1.12) and most of the points are refused due to SamplingD and empty spaces in the bounding box (for a typical human mesh).
3) Incremental changes in the point cloud can't be supported because tree needs reconstruction from scratch and queries'll probably take longer as point count increases.