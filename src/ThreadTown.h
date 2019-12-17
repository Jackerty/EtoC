/*****************************************
* Thread town handles thread work queue. *
*****************************************/
#ifndef _THREAD_TOWN_H_
#define _THREAD_TOWN_H_

#include<stdint.h>
#include<pthread.h>

/*****************************************
* Work function for worker to do/call.   *
*****************************************/
typedef void *(*ThreadTownWork)(void*,void*);
/*****************************************
* Work queue node.                       *
*****************************************/
typedef struct ThreadTownJob{
	struct ThreadTownJob *next;
	ThreadTownWork func;
	void *param;
}ThreadTownJob;

/*********************************************
* Create Thread Town to given Thread town    *
* location which has townpopulation variable *
* set.                                       *
*********************************************/
void buildThreadTown(uint32_t population);
/*********************************************
* Populate town with threads including       *
* caller thread. This functions returns      *
* result of caller worker return value.      *
*********************************************/
void *populateThreadTown(void **byworkerinfo);
/*********************************************
* Destroy Thread Town and all of the memory  *
* remaining at the queue.                    *
* Returns rest of the results of joining     *
* threads (there will be townpopulation-1    *
* array of pointers returned). Results array *
* is dynamiclly allocated and need to be     *
* freed.                                     *
*********************************************/
void** burnThreadTown();
/*********************************************
* Add job to the queue.                      *
*********************************************/
void addThreadTownJob(ThreadTownWork work,void *pram);
/*********************************************
* "Signal" threads to stop. Stop will        *
* checked after worker has finished working. *
*********************************************/
void signalThreadTownToStop();
/*********************************************
* Add job list to the queue unsafelly. No    *
* threading safe check should be made. This  *
* is meant to be used initialize queue.      *
* Before town is populated.                  *
*********************************************/
void addThreadTownJobUnsafe(ThreadTownJob *addition,ThreadTownJob *last);

#endif /* _THREAD_TOWN_H_ */
