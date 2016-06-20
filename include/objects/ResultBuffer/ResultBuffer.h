#ifndef RESULTBUFFER_HEAD__
#define RESULTBUFFER_HEAD__

#include <stdint.h>
#include <stdbool.h>

typedef struct ResultBuffer {
	uint32_t tail; /* size of data that has been written */
	uint32_t size; /* size of ResultBuffer (char data[]) */
	uintptr_t ptr; /* point to next write position */
	char data[0];
} ResultBuffer;

ResultBuffer *constructResultBuffer(uint32_t size);
void destructResultBuffer(ResultBuffer *rb);
void resetResultBuffer(ResultBuffer *rb);
void *getWritePtrResultBuffer(ResultBuffer *rb, uint32_t dsize);

#endif //RESULTBUFFER_HEAD__
