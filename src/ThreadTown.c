/**********************************************
* See ThreadTown.h for description of module. *
**********************************************/
#include<stdint.h>
#include<stdlib.h>
#include<stddef.h>
#include<pthread.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<assert.h>
#include"PrintTools.h"
#include"ThreadTown.h"
#ifdef __STDC_NO_ATOMICS__
	#error Support for the atomics must exist!
#else
	#include<stdatomic.h>
#endif

/***************************************
* Pointer to array of threads.         *
***************************************/
pthread_t *workers;
/***************************************
* Population of the "town" a.k.a       *
* number of threads.                   *
***************************************/
uint32_t townpopulation;

#if defined(THREAD_TOWN_RING)
/**********************************************
* This implementation uses array for queues.  *
* Two rings; one for jobs one for free job    *
* forms.                                      *
* TODO: What happens if there isn't enough    *
*       job slots? realloc?                   *
**********************************************/

#elif defined(THREAD_TOWN_ONE_MUTEX) || 1
/**********************************************
* This implementation is tradional one mutex  *
* that is locked when poping or pushing.      *
* TODO: Isn't using memory well as frees done *
*       jobs rather then having free pile.    *
*       Maybe there is better implementation  * 
*       thought.... without linked list....   *
**********************************************/

/***************************************
* Job queue pointers. One for start    *
* (pop) and one for last (push).       *
***************************************/
ThreadTownJob *jobqueue;
ThreadTownJob *jobqueuelast;
/***************************************
* Mutexes for protecting queues.       *
***************************************/
pthread_mutex_t queuemutex;
/***************************************
* Conditinal for waitng if queue is    *
* empty.                               *
***************************************/
pthread_cond_t queuewaitcondition;
/***************************************
* Stop workers for working.            *
***************************************/
uint8_t stop;
/***************************************
* Number of threads at wating.         *
***************************************/
uint32_t numberofwaiters;

	/***************************************
	* Since there could be threads that    *
	* come conditional after first         *
	* broadcast so we have to look that    *
	* number waiters doesn't go up.        *
	***************************************/
	static void doEvacuation(){
		while(numberofwaiters>1){
			pthread_cond_broadcast(&queuewaitcondition);
			// Sleep after every broadcast to make sure
			// other threads get processing time.
			sleep(1);
		}
	}
	/***************************************
	* Workers waiting function for a jobs. *
	* "Finds" next job for the worker.     *
	* Parameter is the town.               *
	***************************************/
	static void *workFinder(void* workerinfo){
		// Current job pointer.
		ThreadTownJob *currentjob;
		// To make sure that stop isn't called before
		// every thread exists we start with waiters
		// value as high and then reduce that number
		// when workers come to work.
		atomic_fetch_sub(&numberofwaiters,1);

		while(!stop){
			pthread_mutex_lock(&queuemutex);

			while(jobqueue==(ThreadTownJob*)&jobqueue){
				unsigned int count=numberofwaiters++;
				if(count<townpopulation){
					pthread_cond_wait(&queuewaitcondition,&queuemutex);
					numberofwaiters--;
					if(stop){
						pthread_mutex_unlock(&queuemutex);
						return 0;
					}
				}
				else{
					printconst(STDERR_FILENO,"WARMING: Every thread is in conditionalally waiting!");
					numberofwaiters--;
					stop=1;
					pthread_mutex_unlock(&queuemutex);
					doEvacuation();
					return 0;
				}
			}

			currentjob=jobqueue;
			jobqueue=jobqueue->next;

			// if jobqueue is now at the end we need to
			// tells jobqueulast it is pointing to nonsense.
			if(jobqueue==(ThreadTownJob*)&jobqueue){
				jobqueuelast=(ThreadTownJob*)&jobqueue;
			}

			pthread_mutex_unlock(&queuemutex);

			// Execute the job
			//TODO: Returns void*!!!
			currentjob->func(currentjob->param,workerinfo);
			
			//TODO: Implement free pile!
			free(currentjob);
		}
		return 0;
	}
	/*******************
	* See ThreadTown.h *
	*******************/
	void buildThreadTown(uint32_t population){
		pthread_mutex_init(&queuemutex,0);
		pthread_cond_init(&queuewaitcondition,0);
		townpopulation=population;
		// Note that current thread is calculate as member of the townpopulation.
		workers=malloc((townpopulation-1)*sizeof(pthread_t));
		if(workers){
			
			jobqueue=(ThreadTownJob*)&jobqueue;
			jobqueuelast=(ThreadTownJob*)&jobqueue;
		}
		else{
			const char errmsg[]="buildThreadTown | Malloc failed! errno: ";
			char *errnostr=strerror(errno);
			printStrCat3(STDERR_FILENO,errmsg,errnostr,"\n",sizeof(errmsg)-1,strlen(errnostr),1);
		}
	}
	/*******************
	* See ThreadTown.h *
	*******************/
	void *populateThreadTown(void *byworkerinfo,int stride){

		// Make sure that wokers aren't stopping!
		stop=0;
		numberofwaiters=townpopulation+1;

		// Note that current thread is calculate as member of the townpopulation.
		for(uint32_t worker=0;worker<townpopulation-1;worker++){
			if(pthread_create(&workers[worker],0,workFinder,byworkerinfo+(worker*stride))!=0){
				return 0;
			}
		}
		
		return workFinder(byworkerinfo+(townpopulation-1)*stride);

	}
	/*******************
	* See ThreadTown.h *
	*******************/
	void** burnThreadTown(){
		// Allocate return values of the threads.
		void **results=(void**)malloc((townpopulation-1)*sizeof(void*));

		// Make sure that threads are closing.
		// Note that current thread is calculate as member of the townpopulation.
		for(uint32_t worker=0;worker<townpopulation-1;worker++){
			pthread_join(workers[worker],results[worker]);
		}

		// Destroy the thread town
		pthread_mutex_destroy(&queuemutex);
		pthread_cond_destroy(&queuewaitcondition);
		free(workers);

		// Make sure that work queue is empty.
		ThreadTownJob *ite=jobqueue;
		while(ite!=(ThreadTownJob*)&jobqueue){
			ThreadTownJob *rm=ite;
			ite=ite->next;
			free(rm);
		}

		return results;
	}
	/*******************
	* See ThreadTown.h *
	*******************/
	void addThreadTownJob(ThreadTownWork work,void *param){
		// Create the job structure.
		ThreadTownJob* addition=malloc(sizeof(ThreadTownJob));
		addition->next=(ThreadTownJob*)&jobqueue;
		addition->func=work;
		addition->param=param;

		pthread_mutex_lock(&queuemutex);
		jobqueuelast->next=addition;
		jobqueuelast=jobqueuelast->next;
		pthread_cond_signal(&queuewaitcondition);
		pthread_mutex_unlock(&queuemutex);
	}
	/*******************
	* See ThreadTown.h *
	*******************/
	void signalThreadTownToStop(){
		stop=1;
		doEvacuation();
	}
	/*******************
	* See ThreadTown.h *
	*******************/
	void addThreadTownJobUnsafe(ThreadTownJob *addition,ThreadTownJob *last){
		jobqueuelast->next=addition;
		jobqueuelast=last;
		last->next=(ThreadTownJob*)&jobqueue;
	}

#elif defined(THREAD_TOWN_TWO_MUTEX)
/**********************************************
* This is experimental implementation of      *
* having two mutex one for popping and one    *
* for pushing.                                *
* TODO: DOESN'T WORK!!!                       *
**********************************************/

/***************************************
* Job queue pointers. One for start    *
* (pop) and one for last (push).       *
***************************************/
_Atomic(ThreadTownJob *) jobqueue;
_Atomic(ThreadTownJob *) jobqueuelast;
/***************************************
* Mutexes for protecting queues.       *
***************************************/
pthread_mutex_t queueaddmutex;
pthread_mutex_t queuetakemutex;
/***************************************
* Conditinal for waitng if queue is    *
* empty.                               *
***************************************/
pthread_cond_t queuewaitcondition;
/***************************************
* Stop workers for working.            *
***************************************/
atomic_bool stop;
/***************************************
* Number of threads at wating.         *
***************************************/
atomic_uint numberofwaiters;

	/***************************************
	* Since there could be threads that    *
	* come conditional after first         *
	* broadcast so we have to look that    *
	* number waiters doesn't go up.        *
	***************************************/
	static void doEvacuation(){
		while(numberofwaiters>2){
			pthread_cond_broadcast(&queuewaitcondition);
			// Sleep after every broadcast to make sure
			// other threads get processing time.
			sleep(1);
		}
	}
	/***************************************
	* Workers waiting function for a jobs. *
	* "Finds" next job for the worker.     *
	* Parameter is the town.               *
	***************************************/
	static void *workFinder(void* null){

		// Current job pointer.
		ThreadTownJob *currentjob;

		// Check do we stop working!
		while(!stop){
			// We have to check that there is work for
			// this worker. Queues take mutex locked to
			// prevent worker to take same work and in
			// situation where queue is running low.
			pthread_mutex_lock(&queuetakemutex);
			// If there isn't work conditial wait is executed.
			while(jobqueue==(ThreadTownJob*)&jobqueue){
				if(jobqueue->next==(ThreadTownJob*)&jobqueue){
					unsigned int count=atomic_fetch_add(&numberofwaiters,1);

					#ifdef _DEBUG_
          // This is to debug consistency between jobqueue and jobqueuelast
          {
						printf("Jobqueue address: %ld | Jobqueuelast points to %ld\n",(long)&jobqueue,(long)jobqueuelast);
          }
          #endif

					if(count<townpopulation){
						pthread_cond_wait(&queuewaitcondition,&queuetakemutex);
						atomic_fetch_sub(&numberofwaiters,1);
						if(stop){
							pthread_mutex_unlock(&queuetakemutex);
							return 0;
						}
					}
					else{
						printconst(STDERR_FILENO,"WARMING: Every thread is in conditionalally waiting!");
						stop=1;
						pthread_mutex_unlock(&queuetakemutex);
						doEvacuation();
						return 0;
					}
				}
				else{
					atomic_store(&jobqueue,jobqueue->next);
				}
			}

			// We have the work so take work pointer, make sure is isn't queues last and move the pointer.
			currentjob=atomic_load(&jobqueue);

			atomic_store(&jobqueue,currentjob->next);
			//if(jobqueue->next==&jobqueue){
			//	jobqueuelast=&jobqueue;
			//}
			pthread_mutex_unlock(&queuetakemutex);

			// Execute the job
			//TODO: Returns void*!!!
			currentjob->func(currentjob->param);
			// Exiting so free the job structure.
			free(currentjob);
		}

		return 0;
	}
	/*******************
	* See ThreadTown.h *
	*******************/
	void buildThreadTown(uint32_t population){
		townpopulation=population;
		pthread_mutex_init(&queueaddmutex,0);
		pthread_mutex_init(&queueaddmutex,0);
		pthread_cond_init(&queuewaitcondition,0);
		// Note that current thread is calculate as member of the townpopulation.
		workers=malloc((townpopulation-1)*sizeof(pthread_t));
		jobqueue=0;
		jobqueuelast=(ThreadTownJob*)&jobqueue;
	}
	/*******************
	* See ThreadTown.h *
	*******************/
	void *populateThreadTown(){

		// Make sure that wokers aren't stopping!
		stop=0;
		numberofwaiters=1;

		// Note that current thread is calculate as member of the townpopulation.
    for(uint32_t worker=0;worker<townpopulation-1;worker++){
    	if(pthread_create(&workers[worker],0,workFinder,0)!=0){
				return 0;
			}
    }

		return workFinder(0);

	}
	/*******************
	* See ThreadTown.h *
	*******************/
	void **burnThreadTown(){

		// Allocate return values of the threads.
		void **results=(void**)malloc((townpopulation-1)*sizeof(void*));

		// Make sure that threads are closing.
		// Note that current thread is calculate as member of the townpopulation.
		for(uint32_t worker=0;worker<townpopulation-1;worker++){
			pthread_join(workers[worker],results[worker]);
		}

		// Destroy the thread town
		pthread_mutex_destroy(&queueaddmutex);
		pthread_mutex_destroy(&queuetakemutex);
		pthread_cond_destroy(&queuewaitcondition);
		free(workers);

		// Make sure that work queue is empty.
		ThreadTownJob *ite=jobqueue;
		while(ite!=(ThreadTownJob*)&jobqueue){
			ThreadTownJob *rm=ite;
			ite=ite->next;
			free(rm);
		}

		return results;
	}
	/*******************
	* See ThreadTown.h *
	*******************/
	void addThreadTownJob(ThreadTownWork work,void *param){

		// Create the job structure.
		ThreadTownJob* addition=malloc(sizeof(ThreadTownJob));
		addition->next=(ThreadTownJob*)&jobqueue;
		addition->func=work;
		addition->param=param;

		// We have to get queue add mutex to prevent other
		// threads from adding jobs to the queue.
		pthread_mutex_lock(&queueaddmutex);

		// We should be enable just add next since it is only one atomic
		// operation and taker handles so that jobqueuelast points to
		// sensible place.
		atomic_store(&jobqueuelast->next,addition);
		atomic_store(&jobqueuelast,addition);
		pthread_cond_signal(&queuewaitcondition);

		pthread_mutex_unlock(&queueaddmutex);
	}
	/*******************
	* See ThreadTown.h *
	*******************/
	void addThreadTownJobUnsafe(ThreadTownJob *addition,ThreadTownJob *last){
		jobqueuelast->next=addition;
		jobqueuelast=last;
		last->next=(ThreadTownJob*)&jobqueue;
	}
	/*******************
	* See ThreadTown.h *
	*******************/
	void signalThreadTownToStop(){
		stop=1;
		doEvacuation();
	}
#else
/**********************************************
* Fully atomic implementation of the queue.   *
* TODO: Investigate does ABA problem          *
*       solvable.                             *
* TODO: IMPLEMENT!!!                          *
**********************************************/

#endif /* THREAD_TOWN_ONE_MUTEX */
