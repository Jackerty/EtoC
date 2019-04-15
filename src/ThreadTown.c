/**********************************************
* See ThreadTown.h for description of module. *
**********************************************/
#include<pthread.h>
#include"ThreadTown.h"

/***************************************
* Workers waiting function for a jobs. *
***************************************/
void *work

/*******************
* See ThreadTown.h *
*******************/
void buildThreadTown(ThreadTown *town){
	town->lastworkqueue=0;
	pthread_mutex_init(town->queueaddmutex,0);
	pthread_mutex_init(town->queueaddmutex,0);
	town->workers=malloc(town->townpopulation*sizeof(pthread_t));
	town->workqueue=0;
	town->lastworkqueue=(ThreadTownJob*)&town->workqueue;
}
/*******************
* See ThreadTown.h *
*******************/
void burnThreadTown(ThreadTown *town){

	// Make sure that threads are closing.
  for(uint32_t worker=0;worker<town->townpopulation;worker++){
    town->workers[worker];
  }

	// Destroy the thread town
	pthread_mutex_destroy(town->queueaddmutex);
	pthread_mutex_destroy(town->queuetakemutex);
	free(town->workers);

	// Make sure that work queue is empty.
	ThreadTownJob ite=town->workqueue;
	while(ite){
		ThreadTownJob rm=ite;
		ite=ite->next;
		free(ite);
	}
}
