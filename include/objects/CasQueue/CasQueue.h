#ifndef CASQUEUE_HEAD__
#define CASQUEUE_HEAD__

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#if __WORDSIZE == 64
typedef uint64_t casq_refcount_t__;
typedef __uint128_t casq_cellptr_t__;
#else
typedef uint32_t casq_refcount_t__;
typedef uint64_t casq_cellptr_t__;
#endif

/* forward declaration */
struct CasQueueCell;

/*** open definition ***/
typedef union CasQueueCellPointer {  /* 128 bit structure (64 bit) */
	casq_cellptr_t__ val;
	struct {
		struct CasQueueCell *ptr;
		casq_refcount_t__ count;
	};
} CasQueueCellPointer;// __attribute__((may_alias));

typedef struct CasQueueCell {
	volatile union CasQueueCellPointer next;
	void *dptr;
} CasQueueCell;

typedef struct CasQueue {
	volatile CasQueueCellPointer head;
	volatile CasQueueCellPointer tail;
} CasQueue;

extern CasQueue *constructCasQueue(void);
extern void destructCasQueue(CasQueue *cq);

extern void enqCasQueue(CasQueue *cq, void *dptr);
extern bool deqCasQueue(CasQueue *cq, void **dptrp);
extern bool isEmptyCasQueue(CasQueue *cq);


/*** close definition ***/
#ifdef CASQUEUE_SOURCE__
static CasQueueCell *constructCasQueueCell(void *dptr);
static void destructCasQueueCell(CasQueueCell *cqc);
#endif

#endif //CASQUEUE_HEAD__
