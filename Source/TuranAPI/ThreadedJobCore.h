#pragma once
#include "API_includes.h"

/*TuranAPI's C Multi-Threading and Job System Library
* If you created threads before, you can pass them to the Threader. Threader won't create any other thread.
* But you shouldn't destroy any of their objects before destroying Threader, otherwise there will be a dangling pointer issue
* This ThreadPool system is also capable of using main thread as one the threads
*/


TAPIHANDLE(wait)
TAPIHANDLE(threadingsystem)


#ifdef __cplusplus
extern "C" {
#endif
#ifdef TAPI_THREADING_CPP_HELPER
	//Please don't use this function as a user, this is for C++ templates
	void tapi_JobSearch_DONTUSE(tapi_threadingsystem ThreadingSys);
#endif

	unsigned char tapi_GetThisThreadIndex(tapi_threadingsystem ThreadingSys);
	unsigned char tapi_GetThreadCount(tapi_threadingsystem ThreadingSys);

	/*Wait a job; First the thread yields. If it should wait again, executes any other job while waiting
	* Be careful of nonwait-in-wait situations such as;
	You wait for Job B (Thread2) in Job A (Thread1). Job B takes too long, so Thread1 runs Job C here while Job B is running.
	Job C calls Job D and waits for it, so Job D is executed (it doesn't have to start immediately, but in worst case it starts) (Thread3 or Thread1)
	Job D depends on some data that Job B is working on but you forgot to call wait for Job B in Job D.
	So Job D and Job B works concurrently, which are working on the same data.
	* To handle this case, you should either;
	1) Create-store your JobWaitInfos in a shared context (this requires ClearJobWait() operation and planning all your jobs beforehand)
	2) Use WaitJob_empty in Job A, so thread'll keep yielding till job finishes
	*/
	tapi_wait tapi_Create_Wait(tapi_threadingsystem ThreadingSys);
	void tapi_waitJob_busy(tapi_threadingsystem ThreadingSys, tapi_wait wait);
	void tapi_waitJob_empty(tapi_threadingsystem ThreadingSys, tapi_wait wait);
	void tapi_ClearWaitInfo(tapi_threadingsystem ThreadingSys, tapi_wait wait);
	void tapi_waitForAllOtherJobs(tapi_threadingsystem ThreadingSys);

	//If you want to wait for this job, you can pass a JobWaitInfo
	//But if you won't wait for it, don't use it because there may be crashes because of dangling wait reference
	//Critical Note : You shouldn't use same JobWaitInfo object across Execute_withWait()s without ClearJobWait()!
	void tapi_Execute_withWait(tapi_threadingsystem ThreadingSys, void(*func)(), tapi_wait* wait);
	void tapi_Execute_withoutWait(tapi_threadingsystem ThreadingSys, void(*func)());
	void tapi_JobSystem_Start(tapi_threadingsystem* ThreadingSysPTR);
	//If you want to close job system, you should call this instead of Desctructor!
	//This function may run jobs if there is any when it is called
	//So that means, destruction of the job system is the responsibility of the user to handle their jobs
	//I mean, you probably should synchronize all your jobs at one point and check if you should close the job system
	//If you call this while running some other jobs in other threads and they don't care if destructor is called
	//Application may run forever 
	void tapi_CloseJobSystem(tapi_threadingsystem threadingsys);

#ifdef __cplusplus
}
#endif



