#ifndef BUFVECTOR3D_HEAD__
#define BUFVECTOR3D_HEAD__

#include <stdint.h>
#include "Gadget/GadId.h"
#include "Gadget/GadgetHeader.h"

#ifdef BUFVECTOR3D_SOURCE__
/*** private ***/
#define DIMENSION (3)
//#define INITIAL_SIZE (1024LU * 1024 * 1024 * 4)
#define INITIAL_SIZE (1024LU * 1024 * 128)

typedef struct UshortVector3D {
	uint16_t v[DIMENSION];
} UshortVector3D;
typedef struct BufUshortVector3D {
	size_t size;
	unsigned long long int nuvs;
	UshortVector3D uv[0];
} BufUshortVector3D;

typedef struct FloatVector3D {
	float v[DIMENSION];
} FloatVector3D;
typedef struct BufFloatVector3D {
	size_t size;
	unsigned long long int nfvs;
	FloatVector3D fv[0];
} BufFloatVector3D;
#else
/*** public ***/
typedef struct BufUshortVector3D {
	size_t size;
	unsigned long long int nuvs;
} BufUshortVector3D;

typedef struct BufFloatVector3D {
	size_t size;
	unsigned long long int nfvs;
} BufFloatVector3D;
#endif //BUFVECTOR3D_SOURCE__

extern BufUshortVector3D *constructBufUshortVector3D(void);
extern BufUshortVector3D *constructBulkBufUshortVector3D(unsigned long long int n);
extern void destructBufUshortVector3D(BufUshortVector3D *buv);

extern BufUshortVector3D *reallocBufUshortVector3D(BufUshortVector3D *buv, unsigned long long int needed);
extern void resetBufUshortVector3D(BufUshortVector3D *buv);
extern void increaseBufUshortVector3D(BufUshortVector3D *buv, unsigned long long int n);
extern uint16_t *getVectorBufUshortVector3D(BufUshortVector3D *buv, unsigned long long int n);

/* functions for compressed particles */
extern int readCompressedParticlesHeader(char *fname, GadgetHeader *hdr);
extern BufUshortVector3D *readCompressedParticles(BufUshortVector3D *buv, char *fname, GadgetHeader *hdr);
extern BufUshortVector3D *readAndAppendCompressedParticles(BufUshortVector3D *buv, char *fname, GadgetHeader *hdr);
extern void decodeCompressedParticle(uint16_t *uv, float *pos, GadgetHeader *hdr, GadId *idsys, unsigned long long int id,
				 float cell_interval, double unitsep);


extern BufFloatVector3D *constructBufFloatVector3D(void);
extern BufFloatVector3D *constructBulkBufFloatVector3D(unsigned long long int n);
extern void destructBufFloatVector3D(BufFloatVector3D *bfv);

extern BufFloatVector3D *reallocBufFloatVector3D(BufFloatVector3D *bfv, unsigned long long int needed);
extern void resetBufFloatVector3D(BufFloatVector3D *bfv);
extern void increaseBufFloatVector3D(BufFloatVector3D *bfv, unsigned long long int n);
extern float *getVectorBufFloatVector3D(BufFloatVector3D *bfv, unsigned long long int n);

/* functions for decoded particles */
extern BufFloatVector3D *readDecodedParticles(BufFloatVector3D *bfv, char *fname);
extern BufFloatVector3D *readAndAppendDecodedParticles(BufFloatVector3D *bfv, char *fname);
extern BufUshortVector3D *decodeAndOutputCompressedParticles(BufUshortVector3D *buv, char *file_name, int ntile);

#ifdef BUFVECTOR3D_SOURCE__
static size_t GetFileSize(FILE *fp);
#endif// BUFVECTOR3D_SOURCE__

#endif //BUFVECTOR3D_HEAD__
