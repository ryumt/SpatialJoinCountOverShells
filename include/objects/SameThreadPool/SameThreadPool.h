#ifndef SAMETHREADPOOL_HEAD__
#define SAMETHREADPOOL_HEAD__

#include <stdbool.h>
#include <pthread.h>

#include "objects/ThreadPool/ThreadRequest.h"


typedef struct SameThreadPool {
	pthread_t *workers;
	int nworkers;
		
	ThreadRequest request;
	int nrequested;
	int ncompleted;
} SameThreadPool;

extern SameThreadPool *constructSameThreadPool(int nthreads);
extern void destructSameThreadPool(SameThreadPool *stp);
extern void requestWorkSameThreadPool(SameThreadPool *stp, void (*func)(void *), void *arg);
extern void waitCompletionSameThreadPool(SameThreadPool *stp);

#ifdef SAMETHREADPOOL_HEAD__
static void *WorkerRoutine(void *arg);
#endif// SAMETHREADPOOL_HEAD__

#endif //SAMETHREADPOOL_HEAD__
