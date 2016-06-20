#ifndef THREADREQUEST_HEAD__
#define THREADREQUEST_HEAD__

typedef struct ThreadRequest {
        void (*func)(void *);
	void *arg;
} ThreadRequest;

extern ThreadRequest *constructThreadRequest(void (*func)(void *), void *arg);
extern void destructThreadRequest(ThreadRequest *tr);
#endif //THREADPOOL_HEAD__
