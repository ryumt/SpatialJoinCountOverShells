#define CASQUEUE_SOURCE__

#include <stdio.h>
#include <stdlib.h>

#include "objects/CasQueue/CasQueue.h"
#include "atomic.h"


#define CopyCasQueueCellPointer(p1, p2) ((p1)->val = (p2)->val)
#define IsEqualCasQueueCellPointer(p1, p2) ((p1)->val == (p2)->val)
//#define GET_CELL_PTR(x) ((CasQueueCellPointer *)(&x))
//#define CAST_TO_INTEGER_PTR(p) ((casq_compare_t__ *)(p))


inline 
CasQueue *
constructCasQueue(void)
{
	CasQueue *cq;
	CasQueueCell *head;
	
	cq = (void *)malloc(sizeof(*cq));
	head = constructCasQueueCell(NULL);
	cq->head.ptr = cq->tail.ptr = head;
	return cq;
}

inline 
void 
destructCasQueue(CasQueue *cq)
{
	void *dptr;
	
	while (deqCasQueue(cq, &dptr) == true)
		;
	free(cq);
}

inline 
void 
enqCasQueue(CasQueue *cq, void *dptr)
{
	CasQueueCell *cell;
	CasQueueCellPointer old_tail, old_next, p;
	
	cell = constructCasQueueCell(dptr);
        while (true) {
		CopyCasQueueCellPointer(&old_tail, &cq->tail);
		CopyCasQueueCellPointer(&old_next, &cq->tail.ptr->next);
		if (IsEqualCasQueueCellPointer(&old_tail, &cq->tail)) { 
			if (old_next.ptr == NULL) {
				p.ptr = cell;
				p.count = old_next.count + 1;
				
				if (bool_old_compare_and_swap(&old_tail.ptr->next.val, old_next.val, p.val) == true)
					break;
                        }
                        else {
				p.ptr = old_next.ptr;
				p.count = cq->tail.count + 1;
                                bool_old_compare_and_swap(&cq->tail.val, old_tail.val, p.val);
			}
		}
        }
	
	p.ptr = cell;
	p.count = old_tail.count + 1;
	bool_old_compare_and_swap(&cq->tail.val, old_tail.val, p.val);
}

inline 
bool 
deqCasQueue(CasQueue *cq, void **dptrp)
{
	CasQueueCellPointer old_head, old_tail, old_next, p;
	
        while (true) {
		CopyCasQueueCellPointer(&old_head, &cq->head);
		CopyCasQueueCellPointer(&old_tail, &cq->tail);
		
		CopyCasQueueCellPointer(&old_next, &old_head.ptr->next);
		if (IsEqualCasQueueCellPointer(&old_head, &cq->head)) {
			if (old_head.ptr == old_tail.ptr) {
				/* empty */
				if (old_next.ptr == NULL)
                                        return false;
				
				p.ptr = old_next.ptr;
				p.count = cq->tail.count + 1;
                                bool_old_compare_and_swap(&cq->tail.val, old_tail.val, p.val);
			}
                        else {
				*dptrp = old_next.ptr->dptr;
				p.ptr = old_next.ptr;
				p.count = cq->head.count + 1;
				if (bool_old_compare_and_swap(&cq->head.val, old_head.val, p.val) == true)
					break;
                        }
                }
        }
	
	destructCasQueueCell(old_head.ptr);
	return true;
}

inline 
bool 
isEmptyCasQueue(CasQueue *cq)
{
        CasQueueCellPointer old_head, old_tail;
	
	CopyCasQueueCellPointer(&old_head, &cq->head);
	CopyCasQueueCellPointer(&old_tail, &cq->tail);
	return (old_head.ptr == old_tail.ptr);
}

static inline 
CasQueueCell *
constructCasQueueCell(void *dptr)
{
	CasQueueCell *cqc;
	
	cqc = (void *)calloc(1, sizeof(*cqc));
	cqc->dptr = dptr;
	return cqc;
}

static inline 
void 
destructCasQueueCell(CasQueueCell *cqc)
{
	free(cqc);
}
