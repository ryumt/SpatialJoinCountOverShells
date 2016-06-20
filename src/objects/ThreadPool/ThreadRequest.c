#include <stdlib.h>

#include "objects/ThreadPool/ThreadRequest.h"

inline 
ThreadRequest *
constructThreadRequest(void (*func)(void *), void *arg)
{
	ThreadRequest *tr;
	
	tr = (void *)malloc(sizeof(*tr));
	if (tr == NULL)
		return NULL;
	tr->func = func;
	tr->arg = arg;
	return tr;
}

inline 
void 
destructThreadRequest(ThreadRequest *tr)
{
	free(tr);	
}
