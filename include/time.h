#ifndef TIME_HEAD__
#define TIME_HEAD__

#include <sys/time.h>

#define MEASURE_TIME_START() \
{						\
	struct timeval __measure_time_var_1__, __measure_time_var_2__;			\
	gettimeofday(&__measure_time_var_1__, NULL);
	
#define MEASURE_TIME_END_PRINT(msg) \
	gettimeofday(&__measure_time_var_2__, NULL);		\
	printTime(&__measure_time_var_1__, &__measure_time_var_2__, msg);			\
}
#define MEASURE_TIME_END_PRINTMILLIS(msg) \
	gettimeofday(&__measure_time_var_2__, NULL);		\
	printTimeMillis(&__measure_time_var_1__, &__measure_time_var_2__, msg);			\
}


static inline 
void
printTime(struct timeval *begin, struct timeval *end, const char *msg)
{
        long diff;
	
        diff = (end->tv_sec - begin->tv_sec) * 1000 * 1000
                + (end->tv_usec - begin->tv_usec);
	printf("%s: %ld seconds\n", msg, diff / 1000 / 1000);
}

static inline 
void
printTimeMillis(struct timeval *begin, struct timeval *end, const char *msg)
{
        long diff;
	
        diff = (end->tv_sec - begin->tv_sec) * 1000 * 1000
                + (end->tv_usec - begin->tv_usec);
	printf("%s: %ld milli seconds\n", msg, diff / 1000);
}

#endif //TIME_HEAD__
