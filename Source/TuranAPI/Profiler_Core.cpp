#include "Profiler_Core.h"
#include <flatbuffers/flatbuffers.h>

namespace TuranAPI {

	//CODE ALL OF THESE!
	Profiled_Scope::Profiled_Scope() {}
	Profiled_Scope::Profiled_Scope(const char* name) : NAME(name) {
		START_POINT = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
		THREAD_ID = std::hash<std::thread::id>{}(std::this_thread::get_id());
		Is_Recording = true;
		DURATION = new long long;
		ShouldOutput = false;
	}
	Profiled_Scope::Profiled_Scope(const char* name, long long* Output) : DURATION(Output){
		START_POINT = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
		THREAD_ID = std::hash<std::thread::id>{}(std::this_thread::get_id());
		Is_Recording = true;
		ShouldOutput = true;
	}
	Profiled_Scope::~Profiled_Scope() {
		//If the scope was being recorded!
		if (Is_Recording) {
			END_POINT = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
			*DURATION = END_POINT - START_POINT;
			if (!ShouldOutput) {
				std::cout << NAME << " Scope Duration: " << *DURATION << " microseconds (milliseconds/1000)" << std::endl;
				delete DURATION;
			}
			/*
			std::cout << "A Profiled scope is saved!" << std::endl;
			std::cout << "Scope Name:" << NAME << std::endl;
			std::cout << "Scope Start Point: " << START_POINT << std::endl;
			std::cout << "Scope End Point: " << END_POINT << std::endl;
			std::cout << "Scope Thread ID: " << THREAD_ID << std::endl;*/
		}
		//If the scope profiling data is filled from a file!
		else {

		}
	}

	Profiler_System* Profiler_System::SELF = nullptr;
	Profiler_System::Profiler_System() {
		SELF = this;

		START_POINT_PTR = new std::chrono::time_point<std::chrono::steady_clock>;
		*((std::chrono::time_point<std::chrono::steady_clock>*)START_POINT_PTR) = std::chrono::high_resolution_clock::now();

		PROFILED_SCOPEs_vector = new vector<Profiled_Scope>;
	}
	Profiler_System::~Profiler_System() {

	}
	long long Profiler_System::Get_LastScopeDuration() {
		return *(*SELF->PROFILED_SCOPEs_vector)[SELF->PROFILED_SCOPEs_vector->size() - 1].DURATION;
	}

	void Profiler_System::Save_a_ProfiledScope_toSession(const Profiled_Scope& PROFILED_SCOPE) {

	}
}