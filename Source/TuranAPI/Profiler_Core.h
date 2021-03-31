#pragma once
#include "TuranAPI/API_includes.h"


/*
	Create a Profiling system that starts profiling in Begin_Profiling, profiles functions with Profile_Function, profiles scopes with Profile_Scope(name) and ends all profiling systems with End_Profiling!


*/

namespace TuranAPI {
	class TURANAPI Profiled_Scope {
	public:
		bool Is_Recording, ShouldOutput;
		long long START_POINT, END_POINT, THREAD_ID;
		long long* DURATION;
		string NAME;
		//Use this constructor to fill the data later!
		Profiled_Scope();
		//Use these constructor to start profiling a scope!
		Profiled_Scope(const char* name);
		Profiled_Scope(const char* name, long long* Output);

		~Profiled_Scope();
	};



	class TURANAPI Profiler_System {
		void* START_POINT_PTR;
		vector<Profiled_Scope>* PROFILED_SCOPEs_vector;
	public:
		static Profiler_System* SELF;
		Profiler_System();
		~Profiler_System();

		void Save_a_ProfiledScope_toSession(const Profiled_Scope& PROFILED_SCOPE);
		long long Get_LastScopeDuration();
	};
}

#define TURAN_STOP_PROFILING() TuranAPI::Stop_Recording_Session()
#define TURAN_PROFILE_SCOPE(name) TuranAPI::Profiled_Scope ProfilingScope##__LINE__(name)
#define TURAN_PROFILE_SCOPE_O(name, output_ptr) TuranAPI::Profiled_Scope ProfilingScope##__LINE__(name, output_ptr)
#define TURAN_PROFILE_FUNCTION() TURAN_PROFILE_SCOPE(__FUNCSIG__)
#define TURAN_GETDURATION_OFLASTPROFILING TuranAPI::Profiler_System::SELF->Get_LastScopeDuration()
