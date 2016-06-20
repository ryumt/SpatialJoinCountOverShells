#ifndef ARRAYSTR_NODE_HEAD__
#define ARRAYSTR_NODE_HEAD__

#include "types/ulong.h"
#include "objects/RTree/Mbr.h"

#define FIRST_AXIS (1)
#define SECOND_AXIS (0)
#define THIRD_AXIS (2)

#define MAX_ENT (10)
//#define MAX_ENT (20)

typedef struct ArraySTRNode {
	union {
		/* non-leaf node has childs */
		struct {
			/* first child position */
			ulong_t pos;
			/* total number of childs */
			ulong_t len;
		};
		/* leaf node has an id */
		ulong_t id;
	};
	Mbr mbr;
} ArraySTRNode;

extern void adjustMbrArraySTRNode(ArraySTRNode nodes[], ulong_t cur);

#endif //ARRAYSTR_NODE_HEAD__
