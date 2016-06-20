#define BUFHALO_SOURCE__

#include "Gadget/BufHalo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>


inline 
BufHalo *
constructBufHalo(void)
{
	BufHalo *bh;
	
        bh = (void *)malloc(sizeof(*bh) + INITIAL_SIZE);
        if (bh == NULL)
                return NULL;
	bh->size = INITIAL_SIZE;
	bh->nhalos = 0;
        return bh;
}

inline 
void 
destructBufHalo(BufHalo *bh)
{
	free(bh);
}

inline 
BufHalo *
reallocBufHalo(BufHalo *bh, ulong_t needed)
{
	void *ptr = bh;
	
	if ((bh->nhalos + needed) * sizeof(*bh->halo) > bh->size) {
		do {
                        bh->size *= 2;
                } while ((bh->nhalos + needed) * sizeof(*bh->halo) > bh->size);
		
		ptr = (void *)realloc(bh, sizeof(*bh) + bh->size);
                if (ptr == NULL) {
                        fprintf(stderr, "ERROR: memory shortage in %s()\n", __func__);
			fprintf(stderr, "    freed buffer %p\n", bh);
                        destructBufHalo(bh);
                }
	}
	return ptr;
}

inline
void
resetBufHalo(BufHalo *bh)
{
        bh->nhalos = 0;
}

inline
void
increaseBufHalo(BufHalo *bh, unsigned long long int n)
{
	bh->nhalos += n;
}

inline
Halo *
getHaloBufHalo(BufHalo *bh, unsigned long long int n)
{
	return &bh->halo[n];
}



inline 
BufHalo *
readHalos(BufHalo *bh, char *fname)
{
	char tmp[BUFSIZE];
	FILE *fp;
        ulong_t row;
	
	/* open halo file */
        if ((fp = fopen(fname, "r")) == NULL) {
                fprintf(stderr, "ERROR: file %s not found in %s().\n", fname, __func__);
                return NULL;
        }
	/* discard headers */
        for (row = 0; row < N_HEADER_LINES; row++) {
                fgets(tmp, sizeof(tmp), fp);
		if (feof(fp)) {
			fprintf(stderr, "ERROR: file %s does not contain headers in %s().\n", fname, __func__);
			fprintf(stderr, "    freed buffer %p\n", bh);
			destructBufHalo(bh);
			fclose(fp);
			return NULL;		
		}
	}
	/* read halo data: main loop */
	for (row = 0; ; row++) {
                fgets(tmp, sizeof(tmp), fp);
		if (feof(fp))
			break;
		
		if (GetDataFromLineFast(tmp, &bh->halo[row].id, &bh->halo[row].mass, bh->halo[row].point) < 0) {
			fprintf(stderr, "ERROR: contents in file %s may be incorrect in %s().\n", fname, __func__);
			fprintf(stderr, "    freed buffer %p\n", bh);
			destructBufHalo(bh);
			fclose(fp);
			return NULL;
		}
		bh = reallocBufHalo(bh, bh->nhalos + 1);
		bh->nhalos++;
	}
	
        fclose(fp);
        return bh;
}

BufHalo *
readAndFilterHalos(BufHalo *bh, char *fname, char *filname)
{
	FilterIdList *fil;
	ulong_t halo_row;
	ulong_t filter_row;
	bool finished;
	
	ulong_t *tmp_filter;
	char tmp[BUFSIZE];
	FILE *fp;
	
	/* read filtering-id list */
	fil = ConstructFilterIdList(filname);
	
	/* sort filtering-id list, to prepare for following mergejoin */
	tmp_filter = (void *)malloc(sizeof(*tmp_filter) * fil->nfilters);
	if (tmp_filter == NULL) {
		fprintf(stderr, "ERROR: cannot allocate tmporary buffer in %s().\n", __func__);
		fprintf(stderr, "    freed buffer %p\n", bh);
		destructBufHalo(bh);
	}
	RadixSort(fil->filter, tmp_filter, fil->nfilters);
	free(tmp_filter);
	
	/* open halo file */
	if ((fp = fopen(fname, "r")) == NULL) {
                fprintf(stderr, "ERROR: file %s not found in %s().\n", fname, __func__);
		fprintf(stderr, "    freed buffer %p\n", bh);
		destructBufHalo(bh);
                return NULL;
        }	
	/* discard headers */
        for (halo_row = 0; halo_row < N_HEADER_LINES; halo_row++) {
                fgets(tmp, sizeof(tmp), fp);
		if (feof(fp)) {
			fprintf(stderr, "ERROR: file %s does not contain headers in %s().\n", fname, __func__);
			fprintf(stderr, "    freed buffer %p\n", bh);
			fclose(fp);
			destructBufHalo(bh);
			return NULL;		
		}
	}
	/* scan halos & mergejoin with filtering-id list */
	finished = false;
	halo_row = filter_row = 0;
	for (;;) {
		Halo *halo = &bh->halo[halo_row];
		
		/* scan string data */
                fgets(tmp, sizeof(tmp), fp);
		/* no records left, end */
		if (feof(fp))
			break;
		/* convert string to binary data */
		if (GetDataFromLineFast(tmp, &halo->id, &halo->mass, halo->point) < 0) {
			fprintf(stderr, "ERROR: contents in file %s may be incorrect in %s().\n", fname, __func__);
			fprintf(stderr, "    freed buffer %p\n", bh);
			fclose(fp);
			destructBufHalo(bh);
			return NULL;
		}
		
		/* halo id is less than filter id, fetch another halo data */
		if (halo->id < fil->filter[filter_row])
			continue;
		/* proceed filtering-id */
		else if (halo->id > fil->filter[filter_row]) {
			while (halo->id > fil->filter[filter_row]) {
				filter_row++;
				/* all filtering-ids were scanned, end */
				if (filter_row >= fil->nfilters) {
					finished = true;
					break;
				}
			}
		}
		
		/* check if all filtering-ids were scanned */
		if (finished)
			break;
		
		/* record id matched filtering-id */
		if (halo->id == fil->filter[filter_row]) {
			bh = reallocBufHalo(bh, bh->nhalos + 1);
			bh->nhalos++;
			
			halo_row++;
		}
	}
	
        fclose(fp);
        return bh;
}



static inline 
int
GetDataFromLineFast(char *str, ulong_t *idp, float *massp, float point[])
{
	int i;
	
	*idp = strtoul(str, NULL, 10);
	for (i = 0; i < 2; i++) {
		if ((str = strchr(str, ' ')) == NULL)
			return -1;
		str++;
	}	
	
	*massp = strtof(str, NULL);
	for (i = 0; i < 6; i++) {
		if ((str = strchr(str, ' ')) == NULL)
			return -1;
		str++;
	}	
	
	point[0] = strtof(str, NULL);
	if ((str = strchr(str, ' ')) == NULL)
		return -1;
	str++;
	point[1] = strtof(str, NULL);
	if ((str = strchr(str, ' ')) == NULL)
		return -1;
	str++;
	point[2] = strtof(str, NULL);
	
	return 0;
}

static inline 
FilterIdList *
ConstructFilterIdList(char *fname)
{
	FilterIdList *fil;
	ulong_t nfilters;
	
	char tmp[BUFSIZE];
	FILE *fp;
	ulong_t row;
	
	if ((fp = fopen(fname, "r")) == NULL) {
                fprintf(stderr, "ERROR: file %s not found in %s().\n", fname, __func__);
                return NULL;
        }
	
	/* count the number of filtering-ids */
        nfilters = 0;
        for (;;) {
                if (fgets(tmp, sizeof(tmp), fp) == NULL && feof(fp))
                        break;
		if (ferror(fp)) {
			snprintf(tmp, sizeof(tmp), "ERROR: while reading %s in %s()\n", fname, __func__);
                        perror(tmp);
			fclose(fp);
			return NULL;
		}
                nfilters++;
        }
	
	fil = (void *)malloc(sizeof(*fil) + sizeof(*fil->filter) * nfilters);
	
	/* read filtering-ids */
	rewind(fp);
	for (row = 0; row < nfilters; row++) {
		if (fgets(tmp, sizeof(tmp), fp) == NULL && feof(fp))
                        break;
		if (ferror(fp)) {
			snprintf(tmp, sizeof(tmp), "ERROR: while reading %s in %s()\n", fname, __func__);
                        perror(tmp);
			fclose(fp);
			DestructFilterIdList(fil);
			return NULL;
		}
		fil->filter[row] = strtol(tmp, NULL, 10);
	}
	fclose(fp);
	return fil;
}

static inline 
void 
DestructFilterIdList(FilterIdList *fil)
{
	free(fil);
}


#undef __BEGIN_RADIX_SORT__
typedef struct LocalBuffer {
        int tails[NUM_OF_RADIX];
	ulong_t val[NUM_OF_RADIX][B_BUFFER_SIZE];
} LocalBuffer;

static inline
int
GetDigit(ulong_t v, int p)
{
        return (v >> p * D_RADIX_SIZE) & 0xff;
}

static inline
void
CreateHistogram(ulong_t *hist, ulong_t *val, ulong_t ndata, int place)
{
        ulong_t idx;
	
        bzero(hist, sizeof(*hist) * NUM_OF_RADIX);
        for (idx = 0; idx < ndata; idx++)
                hist[GetDigit(val[idx], place)]++;
}

static inline
void
PrefixSum(ulong_t *offset, ulong_t *hist)
{
        ulong_t woff;
        int i;

        woff = 0;
        for (i = 0; i < NUM_OF_RADIX; i++) {
                ulong_t tmp = hist[i];
                offset[i] = woff;
                woff += tmp;
        }
}

static inline
void
RadixSort(ulong_t *val, ulong_t *tmp_val, ulong_t ndata)
{
        ulong_t hist[NUM_OF_RADIX];
        int place;
	
        for (place = 0; place < sizeof(*val); place++) {
                LocalBuffer lb;
                ulong_t idx;
                int i;
		
                CreateHistogram(hist, val, ndata, place);
                PrefixSum(hist, hist);
		
                bzero(lb.tails, sizeof(lb.tails));
                for (idx = 0; idx < ndata; idx++) {
                        int digit = GetDigit(val[idx], place);
                        lb.val[digit][lb.tails[digit]] = val[idx];
			lb.tails[digit]++;
                        if (lb.tails[digit] == B_BUFFER_SIZE) {
                                memcpy(&tmp_val[hist[digit]], lb.val[digit], sizeof(*tmp_val) * B_BUFFER_SIZE);
				lb.tails[digit] = 0;
                                hist[digit] += B_BUFFER_SIZE;
                        }
                }
		for (i = 0; i < NUM_OF_RADIX; i++) {
                        if (lb.tails[i] > 0)
                                memcpy(&tmp_val[hist[i]], lb.val[i], sizeof(*tmp_val) * lb.tails[i]);
		}
                SWAP_MACRO(ulong_t *, &val, &tmp_val);
        }
}
#undef __END_RADIX_SORT__
