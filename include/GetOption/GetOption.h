#ifndef GETOPTION_HEAD__
#define GETOPTION_HEAD__

#include <stdbool.h>

typedef enum ValueType {
	VALUE_TYPE_SHORT, 
	VALUE_TYPE_USHORT, 
	VALUE_TYPE_INT, 
	VALUE_TYPE_UINT, 
	VALUE_TYPE_LONG, 
	VALUE_TYPE_ULONG, 
	VALUE_TYPE_FLOAT, 
	VALUE_TYPE_DOUBLE, 
	VALUE_TYPE_CHAR, 
	VALUE_TYPE_STRING, 
	VALUE_TYPE_STRING_PTR, 
	VALUE_TYPE_BOOL
} ValueType;

extern int getOptionValueFromArgs(int argc, char *argv[], 
				  void *dst, ValueType type, 
				  char *opt, int offset);
extern void getOptionValueRangeFromArgs(int argc, char *argv[],
					int *beg, int *end, char *opt);
extern bool getOptionFlagFromArgs(int argc, char *argv[], char *opt);

#endif //GETOPTION_HEAD__
