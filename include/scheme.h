#ifndef SCHEME_HEAD__
#define SCHEME_HEAD__

#define ENABLE_PTHREAD_JOIN

#define DECODE_PREFIX "decoded_"

#define NCOLS (2) /* key + dat */
#define DIMENSION (3) /* dat[] */

#include <stdint.h>
#include <stdbool.h>
#include "types/ulong.h"

typedef struct Result {
	ulong_t id1;
	ulong_t id2;
} Result;

#endif //SCHEME_HEAD__
