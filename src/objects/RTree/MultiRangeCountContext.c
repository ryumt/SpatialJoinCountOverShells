#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "objects/RTree/MultiRangeCountContext.h"


inline 
MultiRangeCountContext *
constructMultiRangeCountContext(int nrads, float *central, bool alloc_rads, bool alloc_cnts)
{
        MultiRangeCountContext *mrcc;

        mrcc = (void *)malloc(sizeof(*mrcc));
        mrcc->nrads = nrads;
        mrcc->central = central;
	if (alloc_rads)
		mrcc->radiuses = (void *)malloc(sizeof(*mrcc->radiuses) * nrads);
	if (alloc_cnts) {
		mrcc->counts = (void *)malloc(sizeof(*mrcc->counts) * nrads);
		bzero(mrcc->counts, sizeof(*mrcc->counts) * nrads);
	}
	
	return mrcc;
}

inline
void
destructMultiRangeCountContext(MultiRangeCountContext *mrcc, bool free_rads, bool free_cnts)
{
	if (free_rads)
		free(mrcc->radiuses);
	if (free_cnts)
		free(mrcc->counts);
        free(mrcc);
}

inline 
int 
checkConstraintMultiRangeCountContext(MultiRangeCountContext *mrcc)
{
	int i;
	
	for (i = 0; i < mrcc->nrads; i++) {
                if (mrcc->radiuses[i] < 0) {
                        fprintf(stderr, "radius must be greater than 0, but %f.\n", mrcc->radiuses[i]);
                        return -1;
                }
                if (i > 0) {
                        if (mrcc->radiuses[i] < mrcc->radiuses[i - 1]) {
                                fprintf(stderr, "range must be sorted ascending order, but %f -> %f.\n",
                                        mrcc->radiuses[i - 1], mrcc->radiuses[i]);
                                return -1;
                        }
                }
        } 
	return 0;
}
