#define SAMETHREADPOOL_SOURCE__

#include <stdio.h>
#include <stdlib.h>

#include "atomic.h"
#include "objects/SameThreadPool/SameThreadPool.h"

#define START (0)
#define STOP (1)
#define END (2)


SameThreadPool *
constructSameThreadPool(int nthreads)
{
	SameThreadPool *stp;
	int i;
	
	stp = (void *)calloc(1, sizeof(*stp));
	if (stp == NULL)
		return NULL;
		
	stp->nworkers = nthreads;
	stp->workers = (void *)malloc(nthreads * sizeof(*stp->workers));
	if (stp->workers == NULL) {
		free(stp);
		return NULL;
	}
	for (i = 0; i < nthreads; i++) {
		if (pthread_create(&stp->workers[i], NULL, WorkerRoutine, stp) != 0) {
			fprintf(stderr, "ERROR: cannot create threads in %s().\n", __func__);
			exit(1);
		}
	}
	
	return stp;
}

void 
destructSameThreadPool(SameThreadPool *stp)
{
	int i;
	
	stp->nrequested = stp->nworkers + 1;
	waitCompletionSameThreadPool(stp);
	for (i = 0; i < stp->nworkers; i++)
		pthread_join(stp->workers[i], NULL);
	free(stp->workers);
	free(stp);
}

inline 
void 
requestWorkSameThreadPool(SameThreadPool *stp, void (*func)(void *), void *arg)
{
	stp->request.func = func;
	stp->request.arg = arg;
	fetch_and_add(&stp->nrequested, stp->nworkers);
}

inline 
void 
waitCompletionSameThreadPool(SameThreadPool *stp)
{
	wait_while(stp->ncompleted < stp->nworkers);
	fetch_and_add(&stp->ncompleted, -stp->nworkers);
}

static 
void *
WorkerRoutine(void *arg)
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
	pthread_exit(NULL);
}

