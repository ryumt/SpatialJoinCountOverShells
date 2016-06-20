#ifndef SUPPORT_FUNCTIONS_HEAD__
#define SUPPORT_FUNCTIONS_HEAD__

#include <math.h>
#include "types/ulong.h"

#define BOOL2STRING(b) ((b) ? "true" : "false")
#define SWAP_MACRO(type, x, y) do { type tmp = *(x); *(x) = *(y); *(y) = tmp; } while (0)

static inline
ulong_t
UlongCeil(const ulong_t a, const ulong_t b)
{
        return (a + (b - 1)) / b;
}

/* From: https://github.com/ntessore/algo */
static inline 
void 
Logspace(double base, double a, double b, int n, float u[])
{
	double c;
	int i;
	
	/* step size */
	c = (b - a)/(n - 1);
	
	/* fill vector */
	for(i = 0; i < n - 1; ++i)
		u[i] = pow(base, a + i*c);
	
	/* fix last entry to 2^b */
	u[n - 1] = pow(base, b);
}

#endif //SUPPORT_FUNCTIONS_HEAD__
