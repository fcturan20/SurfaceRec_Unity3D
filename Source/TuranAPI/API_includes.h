/*	Turan API supports:
1) File I/O and Data Types and Serialization/Deserialization
2) GLM math library
3) STD libraries
4) Image read/write
5) Run-time Type Introspection (Just enable in VS 2019, no specific code!)
6) IMGUI (Needs to be implemented for each GFX_API)
7) Basic profiler (Only calculates time)

	Future supports:
1) Multi-thread, ECS, Job
2) Networking
3) Compression, Hashing, Algorithms
4) Unit Testing Framework
*/

#pragma once

#define TURANAPI
#define TURAN_DEBUGGING 1
/*
#ifdef WINDOWS10_FORENGINE
#ifdef TURANAPI_BUILD
#define TURANAPI __declspec(dllexport)
#else
#define TURANAPI __declspec(dllimport)
#endif // TURANAPI_BUILD
#else
#error TuranAPI only supports Windows 10 for now!
#endif // WINDOWS10_FORENGINE*/

#include <iostream>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace glm;

//Not used:
#include <thread>
#include <map>

//Delete later in the master branch!
#include <iostream>
#include <vector>
#include <string>
using namespace std;


namespace TuranAPI {
	//Stop the application and ask the user to close or continue!
	//I'm using this to remind myself there are problems while I'm working on another thing!
	void TURANAPI Breakpoint(const char* Breakpoint_Reason = nullptr);

	template<typename T>
	unsigned int TURANAPI Delete_Items_from_vector(vector<T>* vector_to_EraseSomething, vector<bool>* ShouldEraseArray_forEachIndex, unsigned int start_index);

	//Enumeration Functions
	//extern string* Find_TuranAPIEnumsName_byValue(unsigned short Enums_Value);
	//extern TuranAPI_ENUMs Find_TuranAPIEnum_byName(const string& Enum_Name);
	void TURANAPI Empty();
}

//Some basic functionality to do debugging!
#define GET_VARIABLE_NAME(Variable) (#Variable)
#define SLEEP_THREAD(Variable) std::this_thread::sleep_for(std::chrono::seconds(Variable));