#include <stdio.h>
#include <stdlib.h>

#include "atomic.h"
#include "objects/ResultBuffer/ResultBuffer.h"

inline 
ResultBuffer *
constructResultBuffer(uint32_t size)
{
	ResultBuffer *rb;
	
	rb = (void *)malloc(sizeof(*rb) + size);
	rb->size = size;
	rb->tail = 0;
	rb->ptr = (uintptr_t)rb->data;
	return rb;
}

inline 
void 
destructResultBuffer(ResultBuffer *rb)
{
	free(rb);
}

inline 
void 
resetResultBuffer(ResultBuffer *rb)
{
	rb->tail = 0;
	rb->ptr = (uintptr_t)rb->data;
}

inline 
void *
getWritePtrResultBuffer(ResultBuffer *rb, uint32_t dsize)
{
	if (fetch_and_add(&rb->tail, dsize) + dsize >= rb->size) {
		fetch_and_add(&rb->tail, -dsize);
		return NULL;
	}
	return (void *)fetch_and_add(&rb->ptr, dsize);
}

