#ifndef THREADPOOL_HEAD__
#define THREADPOOL_HEAD__

#include <pthread.h>

#include "objects/ThreadPool/ThreadRequest.h"
#include "objects/SameThreadPool/SameThreadPool.h"

#ifdef THREADPOOL_SOURCE__
#include "objects/CasQueue/CasQueue.h"
typedef struct ThreadPool {
	CasQueue *requeue;
	pthread_t *workers;
	int nworkers;
	
	int nrequested;
	int ncompleted;
} ThreadPool;
#else
typedef struct ThreadPool {
	void *requeue;
	pthread_t *workers;
	int nworkers;
	
	int nrequested;
	int ncompleted;
} ThreadPool;
#endif //THREADPOOL_SOURCE__


extern ThreadPool *constructThreadPool(int nthreads);
extern void destructThreadPool(ThreadPool *tp);
extern int requestWorkThreadPool(ThreadPool *tp, void (*func)(void *), void *arg);
extern void waitCompletionThreadPool(ThreadPool *tp);

#ifdef THREADPOOL_SOURCE__
static void *WorkerRoutine(void *arg);
#endif //THREADPOOL_SOURCE__


extern SameThreadPool *constructPseudoSameThreadPool(int n);
extern void destructPseudoSameThreadPool(SameThreadPool *stp);
extern void moveThreadPoolWorkerToPseudoSameThreadPool(void *arg);
extern void callbackThreadPoolWorkerFromPseudoSameThreadPool(SameThreadPool *stp);

#endif //THREADPOOL_HEAD__
