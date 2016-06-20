#ifndef BUFHALO_HEAD__
#define BUFHALO_HEAD__

#include <stdbool.h>
#include "types/ulong.h"
#include "support_functions.h"

#define N_HEADER_LINES (16)

#ifdef BUFHALO_SOURCE__
#define INITIAL_SIZE (1024 * 1024 * 32)
#define BUFSIZE (1024 * 16)

#define D_RADIX_SIZE (8)
#define NUM_OF_RADIX (0x01 << D_RADIX_SIZE)
#define B_BUFFER_SIZE (32)

typedef struct FilterIdList {
        ulong_t nfilters;
        ulong_t filter[0];
} FilterIdList;
#endif //BUFHALO_SOURCE__

#define DIMENSION (3)

typedef struct Halo {
        ulong_t id;
        float mass;
        float point[DIMENSION];
} Halo;

typedef struct BufHalo {
	size_t size;
	ulong_t nhalos;
	Halo halo[0];
} BufHalo;

#undef DIMENSION

extern BufHalo *constructBufHalo(void);
extern void destructBufHalo(BufHalo *bh);
extern BufHalo *reallocBufHalo(BufHalo *bh, ulong_t needed);

extern void resetBufHalo(BufHalo *bh);
extern void increaseBufHalo(BufHalo *bh, unsigned long long int n);
extern Halo *getHaloBufHalo(BufHalo *bh, unsigned long long int n);

extern BufHalo *readHalos(BufHalo *bh, char *fname);
extern BufHalo *readAndFilterHalos(BufHalo *bh, char *fname, char *filname);


#ifdef BUFHALO_SOURCE__
static int GetDataFromLineFast(char *str, ulong_t *idp, float *massp, float point[]);
static FilterIdList *ConstructFilterIdList(char *fname);
static void DestructFilterIdList(FilterIdList *fil);

static int GetDigit(ulong_t v, int p);
static void CreateHistogram(ulong_t *hist, ulong_t *val, ulong_t ndata, int place);
static void PrefixSum(ulong_t *offset, ulong_t *hist);
static void RadixSort(ulong_t *val, ulong_t *tmp_val, ulong_t ndata);
#endif //BUFHALO_SOURCE__

#endif //BUFHALO_HEAD__
