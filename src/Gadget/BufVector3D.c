#define BUFVECTOR3D_SOURCE__

#include <stdio.h>
#include <stdlib.h>
#include <x86intrin.h>

#include "scheme.h"
#include "Gadget/BufVector3D.h"


inline 
BufUshortVector3D *
constructBufUshortVector3D(void)
{
	BufUshortVector3D *buv;
	
	buv = (void *)malloc(sizeof(*buv) + INITIAL_SIZE);
	if (buv == NULL)
		return NULL;
	buv->size = INITIAL_SIZE;
	buv->nuvs = 0;
	return buv;
}

inline 
BufUshortVector3D *
constructBulkBufUshortVector3D(unsigned long long int n)
{
	BufUshortVector3D *buv;
	
	buv = (void *)malloc(sizeof(*buv) + sizeof(*buv->uv) * n);
	if (buv == NULL)
		return buv;
	buv->size = sizeof(*buv->uv) * n;
	buv->nuvs = 0;
	return buv;
}

inline 
void 
destructBufUshortVector3D(BufUshortVector3D *buv)
{
	free(buv);	
}

inline 
BufUshortVector3D *
reallocBufUshortVector3D(BufUshortVector3D *buv, unsigned long long int needed)
{
	void *ptr = buv;
	
	if ((buv->nuvs + needed) * sizeof(*buv->uv) > buv->size) {
		do {
			buv->size *= 2;
		} while ((buv->nuvs + needed) * sizeof(*buv->uv) > buv->size);
		
		ptr = (void *)realloc(buv, sizeof(*buv) + buv->size);
		if (ptr == NULL) {
			fprintf(stderr, "ERROR: memory shortage in %s()\n", __func__);
			fprintf(stderr, "    freed buffer %p\n", buv);
			destructBufUshortVector3D(buv);
		}
	}
	return ptr;
}

inline 
void 
resetBufUshortVector3D(BufUshortVector3D *buv)
{
	buv->nuvs = 0;
}

inline 
void 
increaseBufUshortVector3D(BufUshortVector3D *buv, unsigned long long int n)
{
	buv->nuvs += n;
}

inline 
uint16_t *
getVectorBufUshortVector3D(BufUshortVector3D *buv, unsigned long long int n)
{
	UshortVector3D *uv = buv->uv + n;
	return uv->v;
}



inline 
int 
readCompressedParticlesHeader(char *fname, GadgetHeader *hdr)
{
	FILE *fp;
	int dummy;
	
	fp = fopen(fname, "r");
	if (fp == NULL) {
		fprintf(stderr, "ERROR: file %s not found in %s()\n", fname, __func__);
		return -1;
        }
	fseek(fp, sizeof(dummy), SEEK_CUR);
	fread(hdr, sizeof(*hdr), 1, fp);
	fseek(fp, sizeof(dummy), SEEK_CUR);
	fclose(fp);
	return 0;
}


inline 
BufUshortVector3D * 
readCompressedParticles(BufUshortVector3D *buv, char *fname, GadgetHeader *hdr)
{
	FILE *fp;
	int dummy;
	
	fp = fopen(fname, "r");
	if (fp == NULL) {
		fprintf(stderr, "ERROR: file %s not found in %s()\n", fname, __func__);
		fprintf(stderr, "    freed buffer %p\n", buv);
		destructBufUshortVector3D(buv);
		return NULL;
        }
	
	fseek(fp, sizeof(dummy), SEEK_CUR);
	fread(hdr, sizeof(*hdr), 1, fp);
	fseek(fp, sizeof(dummy) * 2, SEEK_CUR);
	
	buv = reallocBufUshortVector3D(buv, hdr->npart[1]);
	fread(buv->uv, sizeof(*buv->uv), hdr->npart[1], fp);
	increaseBufUshortVector3D(buv, hdr->npart[1]);
	
	fclose(fp);
	return buv;
}

inline 
BufUshortVector3D * 
readAndAppendCompressedParticles(BufUshortVector3D *buv, char *fname, GadgetHeader *hdr)
{
	FILE *fp;
	int dummy;
	uint16_t *dstuv;
	
	fp = fopen(fname, "r");
	if (fp == NULL) {
		fprintf(stderr, "ERROR: file %s not found in %s()\n", fname, __func__);
		fprintf(stderr, "    freed buffer %p\n", buv);
		destructBufUshortVector3D(buv);
                return NULL;
        }
	
	fseek(fp, sizeof(dummy), SEEK_CUR);
	fread(hdr, sizeof(*hdr), 1, fp);
	fseek(fp, sizeof(dummy) * 2, SEEK_CUR);
	
	buv = reallocBufUshortVector3D(buv, buv->nuvs + hdr->npart[1]);
	dstuv = getVectorBufUshortVector3D(buv, buv->nuvs);
	fread(dstuv, sizeof(*buv->uv), hdr->npart[1], fp);
	increaseBufUshortVector3D(buv, hdr->npart[1]);
	
	fclose(fp);
	return buv;
}


inline 
void 
decodeCompressedParticle(uint16_t *uv, float *pos, GadgetHeader *hdr, GadId *idsys, unsigned long long int id, 
		     float cell_interval, double unitsep)
{
	int compressed_ip[4] = {
		uv[0], uv[1], uv[2],
	};
	int ip[4];
	
	/* IDをx,y,zのグリッド座標に。 やはりIDは1から始まることに注意。 */
	id2posGadId(idsys, id, &ip[0], &ip[1], &ip[2]);
#if defined(USE_AVX) && defined(__FMAINTRIN_H)
	{
		double unitsep_d = unitsep;
		float unitsep = unitsep_d;
		__m128 v_cellint = _mm_broadcast_ss(&cell_interval);
		__m128i v_ip = (__m128i)_mm_load_ps((float *)ip);
		__m128 v_dispmin = _mm_broadcast_ss(&hdr->disp_min);
		__m128 v_unitsep = _mm_broadcast_ss(&unitsep);
		__m128i v_compip = (__m128i)_mm_load_ps((float *)compressed_ip);
		__m128 v_pos;
		static const float v_diff[4] = {UPP_BOUND, UPP_BOUND, UPP_BOUND, UPP_BOUND};
		
		v_pos = _mm_fmadd_ps(v_unitsep, _mm_cvtepi32_ps(v_compip),
				     _mm_fmadd_ps(v_cellint, _mm_cvtepi32_ps(v_ip), v_dispmin));
		_mm_store_ps(pos, v_pos);
	}
#else 
	pos[0] = cell_interval * ip[0] + hdr->disp_min + unitsep * compressed_ip[0];
	pos[1] = cell_interval * ip[1] + hdr->disp_min + unitsep * compressed_ip[1];
	pos[2] = cell_interval * ip[2] + hdr->disp_min + unitsep * compressed_ip[2];
#endif 
}




inline 
BufFloatVector3D *
constructBufFloatVector3D(void)
{
	BufFloatVector3D *bfv;
	
	bfv = (void *)malloc(sizeof(*bfv) + INITIAL_SIZE);
	if (bfv == NULL)
		return NULL;
	bfv->size = INITIAL_SIZE;
	bfv->nfvs = 0;
	return bfv;
}

inline 
BufFloatVector3D *
constructBulkBufFloatVector3D(unsigned long long int n)
{
	BufFloatVector3D *bfv;
	
	bfv = (void *)malloc(sizeof(*bfv) + sizeof(*bfv->fv) * n);
	if (bfv == NULL)
		return bfv;
	bfv->size = sizeof(*bfv->fv) * n;
	bfv->nfvs = 0;
	return bfv;
}

inline 
void 
destructBufFloatVector3D(BufFloatVector3D *bfv)
{
	free(bfv);	
}

inline 
BufFloatVector3D *
reallocBufFloatVector3D(BufFloatVector3D *bfv, unsigned long long int needed)
{
	void *ptr = bfv;
	
	if ((bfv->nfvs + needed) * sizeof(*bfv->fv) > bfv->size) {
		do {
			bfv->size *= 2;
		} while ((bfv->nfvs + needed) * sizeof(*bfv->fv) > bfv->size);
		
		ptr = (void *)realloc(bfv, sizeof(*bfv) + bfv->size);
		if (ptr == NULL) {
			fprintf(stderr, "ERROR: memory shortage in %s()\n", __func__);
			fprintf(stderr, "    freed buffer %p\n", bfv);
			destructBufFloatVector3D(bfv);
		}
	}
	return ptr;
}

inline 
void 
resetBufFloatVector3D(BufFloatVector3D *bfv)
{
	bfv->nfvs = 0;
}

inline 
void 
increaseBufFloatVector3D(BufFloatVector3D *bfv, unsigned long long int n)
{
	bfv->nfvs += n;
}

inline 
float *
getVectorBufFloatVector3D(BufFloatVector3D *bfv, unsigned long long int n)
{
	FloatVector3D *fv = bfv->fv + n;
	return fv->v;
}


inline 
BufFloatVector3D * 
readDecodedParticles(BufFloatVector3D *bfv, char *fname)
{
	FILE *fp;
	size_t size, num;
	
	fp = fopen(fname, "r");
	if (fp == NULL) {
		fprintf(stderr, "ERROR: file %s not found in %s()\n", fname, __func__);
		fprintf(stderr, "    freed buffer %p\n", bfv);
		destructBufFloatVector3D(bfv);
                return NULL;
        }
	
	size = GetFileSize(fp);
	num = size / sizeof(*bfv->fv);
	bfv = reallocBufFloatVector3D(bfv, num);
	fread(bfv->fv, sizeof(*bfv->fv), num, fp);
	increaseBufFloatVector3D(bfv, num);
	
	fclose(fp);
	return bfv;
}

inline 
BufFloatVector3D * 
readAndAppendDecodedParticles(BufFloatVector3D *bfv, char *fname)
{
	FILE *fp;
	size_t size, num;
	float *dstfv;
	
	fp = fopen(fname, "r");
	if (fp == NULL) {
		fprintf(stderr, "ERROR: file %s not found in %s()\n", fname, __func__);
		fprintf(stderr, "    freed buffer %p\n", bfv);
		destructBufFloatVector3D(bfv);
                return NULL;
        }
	
	size = GetFileSize(fp);
	num = size / sizeof(*bfv->fv);
	bfv = reallocBufFloatVector3D(bfv, bfv->nfvs + num);
	dstfv = getVectorBufFloatVector3D(bfv, bfv->nfvs);
	fread(dstfv, sizeof(*bfv->fv), num, fp);
	increaseBufFloatVector3D(bfv, num);
	
	fclose(fp);
	return bfv;
}


inline 
BufUshortVector3D *
decodeAndOutputCompressedParticles(BufUshortVector3D *buv, char *file_name, int ntile)
{
	GadgetHeader hdr;
	GadgetDecodeParams dparams;
	GadId idsys;
	
	char out_file_name[FILENAME_MAX];
	FILE *fp;
	unsigned long long int n;
	
	
	/* read compressed particles */
	buv = readCompressedParticles(buv, file_name, &hdr);
	/* prepare for decode */
	initGadgetDecodeParams(&dparams, &hdr, ntile);
	initGadId(&idsys, ntile, dparams.nparticles_partile);
	
	/* create output decoded file */
	snprintf(out_file_name, sizeof(out_file_name), "%s%s", DECODE_PREFIX, file_name);
	fp = fopen(out_file_name, "wb");
	if (fp == NULL) {
		fprintf(stderr, "ERROR: cannot open %s in %s()\n", out_file_name, __func__);
		fprintf(stderr, "    freed buffer %p\n", buv);
		destructBufUshortVector3D(buv);
		return NULL;
	}
	
	/* decode main loop */
	for(n = 0; n < buv->nuvs; n++) {
		uint16_t *uv = getVectorBufUshortVector3D(buv, n);
		float pos[4];
		
		decodeCompressedParticle(uv, pos, &hdr, &idsys, n + dparams.id_start, dparams.cell_interval, dparams.unitsep);
		
		fwrite(pos, sizeof(*pos), 3, fp);
		if (ferror(fp)) {
			char msg[FILENAME_MAX * 10];
			
			snprintf(msg, sizeof(msg), "ERROR: while writing %s in %s()\n", out_file_name, __func__);
			perror(msg);
			fprintf(stderr, "    freed buffer %p\n", buv);
			fclose(fp);
			destructBufUshortVector3D(buv);
			return NULL;
		}
		// printf("%lu: {%.10f, %.10f, %.10f}\n", n, pos[0], pos[1], pos[2]);
	}
	resetBufUshortVector3D(buv);
	return 0;
}

static inline 
size_t 
GetFileSize(FILE *fp)
{
	size_t size;
	
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	return size;
}
