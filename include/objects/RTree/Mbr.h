#ifndef RTREE_MBR_HEAD__
#define RTREE_MBR_HEAD__

#include <stdint.h>
#include <stdbool.h>

#define NDIMS (3)

#define LOW_BOUND (0.0)
#define UPP_BOUND (1000.0)

/* 32 byte ((4*3 + 4) byte * 2) */
typedef struct Mbr {
	float upp[NDIMS] __attribute__((__aligned__(16)));
        float low[NDIMS] __attribute__((__aligned__(16)));
} Mbr;

extern float calculateMarginMbr(Mbr *mbr);
extern float calculateAreaMbr(Mbr *mbr);
extern float calculateOverlapAreaMbr(Mbr *mbr1, Mbr *mbr2);
extern float calculateCenterDistanceMbr(Mbr *mbr1, Mbr *mbr2);
extern void enlargeMbr(Mbr *mbr, Mbr *expand);

extern bool checkOverlapMbr(Mbr *mbr1, Mbr *mbr2);
extern void setQueryMbr(Mbr *qmbr, float *central, float radius);
extern bool checkOutOfBoundMbr(Mbr *mbr, float *central, float *mirrored_central);

extern void checkMinimumOutOfBoundMbr(Mbr *mbr, uint32_t *oob_flags);
extern void checkMaximumOutOfBoundMbr(Mbr *mbr, uint32_t *oob_flags);
extern void checkMinMaxOutOfBoundMbr(Mbr *mbr, uint32_t *min_oob, uint32_t *max_oob);

#endif //RTREE_MBR_HEAD__
