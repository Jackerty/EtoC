/*****************************************
* Thread town handles thread work queue. *
*****************************************/
#include<pthread.h>

/*****************************************
* Work function for worker to do/call.   *
*****************************************/
typedef int ThreadTownWork(void*);
/*****************************************
* Work queue node.                       *
*****************************************/
typedef struct ThreadTownJob{
	struct ThreadTownWork *next;
	ThreadTownWork *func;
	void *data;
	int priority;
}ThreadTownJob;
/*****************************************
* Thread town structure manages thread   *
* workers in the "town".                 *
*****************************************/
typedef struct ThreadTown{
	ThreadTownJob *workqueue;
	ThreadTownJob *lastworkqueue;
	pthread_t *workers;
  pthread_mutex_t queueaddmutex;
  phtread_mutex_t queuetakemutex;
  uint32_t townpopulation;
}ThreadTown;

/*********************************************
* Create Thread Town to given Thread town    *
* location which has townpopulation variable *
* set.                                       *
*********************************************/
void buildThreadTown(ThreadTown *town);
/*********************************************
* Destroy Thread Town and all of the memory  *
* remaining at the queue.                    *
*********************************************/
void burnThreadTown(ThreadTown *town);
/*********************************************
*  *
*********************************************/
