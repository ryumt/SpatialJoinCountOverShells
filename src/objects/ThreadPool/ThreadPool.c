#define THREADPOOL_SOURCE__

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "atomic.h"
#include "objects/ThreadPool/ThreadPool.h"


ThreadPool *
constructThreadPool(int nthreads)
{
	ThreadPool *tp;
	int i;
	
	tp = (void *)malloc(sizeof(*tp));
	if (tp == NULL)
		return NULL;
	
	tp->requeue = constructCasQueue();
	tp->nrequested = tp->ncompleted = 0;
	tp->nworkers = nthreads;
	tp->workers = (void *)malloc(nthreads * sizeof(*tp->workers));
	if (tp->workers == NULL) {
		free(tp);
		return NULL;
	}
	for (i = 0; i < nthreads; i++) {
		if (pthread_create(&tp->workers[i], NULL, WorkerRoutine, tp) != 0) {
			fprintf(stderr, "ERROR: cannot create threads in %s().\n", __func__);
			exit(1);
		}
	}
	
	return tp;
}

void 
destructThreadPool(ThreadPool *tp)
{
	int i;
	
	waitCompletionThreadPool(tp);
	for (i = 0; i < tp->nworkers; i++) {
		if (requestWorkThreadPool(tp, NULL, NULL) < 0)
			fprintf(stderr, "ERROR: failed to request termination in %s().\n", __func__);
	}
	for (i = 0; i < tp->nworkers; i++)
		pthread_join(tp->workers[i], NULL);
	free(tp->workers);
	destructCasQueue(tp->requeue);
	free(tp);
}

inline 
int 
requestWorkThreadPool(ThreadPool *tp, void (*func)(void *), void *arg)
{
	ThreadRequest *tr;
	
	tr = constructThreadRequest(func, arg);
	if (tr == NULL) {
		fprintf(stderr, "ERROR: failed to request work in %s().\n", __func__);
		return -1;
	}
	fetch_and_add(&tp->nrequested, 1);
	enqCasQueue(tp->requeue, tr);
	return 0;
}

inline 
void 
waitCompletionThreadPool(ThreadPool *tp)
{
	wait_while(tp->nrequested != tp->ncompleted);
}

static 
void *
WorkerRoutine(void *arg)
{
	ThreadPool *tp = arg;
	
	for (;;) {
		ThreadRequest *request;
		
		/* wait until any requests will arrive */
		while (deqCasQueue(tp->requeue, (void **)&request) == false)
			;
		/* terminate sign */
		if (request->func == NULL)
			break;
		request->func(request->arg);
		destructThreadRequest(request);
		fetch_and_add(&tp->ncompleted, 1);
	}
	fetch_and_add(&tp->ncompleted, 1);
	pthread_exit(NULL);
}



inline
SameThreadPool *
constructPseudoSameThreadPool(int n)
{
        SameThreadPool *stp;
        stp = (void *)calloc(1, sizeof(*stp));
	if (stp == NULL)
		return NULL;
	stp->nworkers = n;
	return stp;
}

inline
void
destructPseudoSameThreadPool(SameThreadPool *stp)
{
        free(stp);
}

/* ThreadPool worker routine */
void 
moveThreadPoolWorkerToPseudoSameThreadPool(void *arg)
{
        SameThreadPool *stp = arg;
	
	for (;;) {
		wait_while(stp->nrequested <= 0);
		if (stp->nrequested > stp->nworkers)
			break;
		
		if (fetch_and_add(&stp->nrequested, -1) <= 0) {
			fetch_and_add(&stp->nrequested, 1);
			continue;
		}
		stp->request.func(stp->request.arg);
		fetch_and_add(&stp->ncompleted, 1);
	}
	fetch_and_add(&stp->ncompleted, 1);
}

/* not ThreadPool worker routine */
inline 
void 
callbackThreadPoolWorkerFromPseudoSameThreadPool(SameThreadPool *stp)
{
	stp->nrequested = stp->nworkers + 1;
	// waitCompletionSameThreadPool(stp);
}

