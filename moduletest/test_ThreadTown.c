/*************************************************
* Program for testing thread town module will    *
* work without exlusion problems.                *
*************************************************/
#include<unistd.h>
#include<stdatomic.h>
#include<string.h>
#include<stdlib.h>
#include"../src/ThreadTown.h"

/******************************************
* Amount of memory in increameting array. *
******************************************/
#define MAX_INCARRAY 4096

/******************************************
* Prime number array to make possibility  *
* that threads don't go at the same time. *
******************************************/
const uint32_t primes[]={2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107,109,113};

/***************************
* Structure given to each  *
* reqioninc call.          *
***************************/
struct DataSample{
	uint32_t count;
	uint32_t tasknumber;
	atomic_uint taskcount;
	uint32_t *incarray;
};

/***************************
* Count the tasks that are *
* done!                    *
***************************/
atomic_uint taskdone;

void *reqioninc(void* param){
	struct DataSample *data=(struct DataSample *)param;
	for(uint32_t i=0;i<data->count;i++) atomic_fetch_add(&data->incarray[i],1);
	unsigned int count=atomic_fetch_add(&(data->taskcount),1);
	if(count<4095) addThreadTownJob(reqioninc,param);
	else{
		uint32_t shouldstop=atomic_fetch_add(&taskdone,1);
		if(shouldstop>=data->tasknumber){
				signalThreadTownToStop();
		}
	}
	return 0;
}

int main(){

	uint32_t cpucount=(uint32_t)sysconf(_SC_NPROCESSORS_ONLN);
  buildThreadTown(cpucount);

	uint32_t incarray[MAX_INCARRAY];
	memset(incarray,0,MAX_INCARRAY*sizeof(uint32_t));
	taskdone=1;
	// We can't allocate array of jobs because they are freed one out of time
	ThreadTownJob **initqueue=malloc(sizeof(ThreadTownJob*)*cpucount*2);
	struct DataSample *data=malloc(sizeof(struct DataSample)*cpucount*2);
	uint32_t runningsum=0;
	for(uint32_t i=0;i<cpucount*2;i++){
		data[i].count=primes[i];
		data[i].tasknumber=cpucount*2;
		data[i].incarray=incarray+runningsum;
		runningsum+=primes[i];
		data[i].taskcount=0;
		initqueue[i]=malloc(sizeof(ThreadTownJob));
		initqueue[i]->func=reqioninc;
		initqueue[i]->param=data+i;
	}
	for(uint32_t i=0;i<cpucount*2-1;i++) initqueue[i]->next=initqueue[i+1];

	addThreadTownJobUnsafe(initqueue[0],initqueue[cpucount*2-1]);
	free(initqueue);
	populateThreadTown();

	void **rm=burnThreadTown();
	free(rm);
}
