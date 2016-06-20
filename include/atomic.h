#ifndef ATOMIC_HEAD__
#define ATOMIC_HEAD__

#include <x86intrin.h>

#define fetch_and_increment(x) __sync_fetch_and_add(x, 1)
#define fetch_and_add(x, v) __sync_fetch_and_add(x, v)
#define bool_compare_and_swap(x, v) __sync_bool_compare_and_swap(x, *x, v)
#define bool_old_compare_and_swap(x, y, v) __sync_bool_compare_and_swap(x, y, v)
#define compare_and_swap(x, v) do {					\
		while (!__sync_bool_compare_and_swap(x, *x, v))		\
			;						\
	} while (0)


#define wait_while(c) do {			\
		while (c)			\
			_mm_pause();		\
	} while (0)
// __sync_synchronize();       
//_mm_pause();	

#endif //ATOMIC_HEAD__
