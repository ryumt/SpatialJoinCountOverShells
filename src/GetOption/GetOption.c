#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "GetOption/GetOption.h"

inline 
int 
getOptionValueFromArgs(int argc, char *argv[], 
		       void *dst, ValueType type, 
		       char *opt, int offset)
{
	int optlen;
	int i;
	
	optlen = strlen(opt);
	for (i = 1; i < argc; i++) {
		if (strlen(argv[i]) != optlen)
			continue;
		if (memcmp(argv[i], opt, optlen) == 0) {
			i += offset + 1;
			if (i >= argc)
				return -1;
			
			switch (type) {
			case VALUE_TYPE_SHORT: 
				*(short *)dst = strtol(argv[i], NULL, 10);
				break;
			case VALUE_TYPE_USHORT: 
				*(unsigned short *)dst = strtol(argv[i], NULL, 10);
				break;				
			case VALUE_TYPE_INT: 
				*(int *)dst = strtol(argv[i], NULL, 10);
				break;
			case VALUE_TYPE_UINT: 
				*(unsigned int *)dst = strtol(argv[i], NULL, 10);
				break;
			case VALUE_TYPE_LONG: 
				*(long *)dst = strtol(argv[i], NULL, 10);
				break;
			case VALUE_TYPE_ULONG: 
				*(unsigned long *)dst = strtoul(argv[i], NULL, 10);
				break;
			case VALUE_TYPE_FLOAT: 
				*(float *)dst = strtod(argv[i], NULL);
				break;
			case VALUE_TYPE_DOUBLE: 
				*(double *)dst = strtod(argv[i], NULL);
				break;
			case VALUE_TYPE_CHAR: 
				*(char *)dst = *(char *)argv[i];
				break;
			case VALUE_TYPE_STRING: 
				memcpy((char *)dst, argv[i], strlen(argv[i]) + 1);
				break;
			case VALUE_TYPE_STRING_PTR: 
				*(char **)dst = argv[i];
				break;
			case VALUE_TYPE_BOOL: 
				*(bool *)dst = (*(char *)dst == 't') ? true : false;
				break;
			default: 
				fprintf(stderr, "Invalid ValueType.\n");
				return -1;
			};
		}
	}
	return 0;
}


inline 
void 
getOptionValueRangeFromArgs(int argc, char *argv[], 
			    int *beg, int *end, char *opt)
{
	int optlen;
	int i, j;
	
	optlen = strlen(opt);
	for (i = 1; i < argc; i++) {
		if (strlen(argv[i]) != optlen)
			continue;
		if (memcmp(argv[i], opt, optlen) == 0) {
			for (j = i + 1; j < argc && *argv[j] != '-'; j++)
				;
			if (j != i + 1) {
				*beg = i + 1;
				*end = j;
			}
		}
	}
}

inline 
bool 
getOptionFlagFromArgs(int argc, char *argv[], char *opt)
{
	int optlen;
	int i;
	
	optlen = strlen(opt);
	for (i = 1; i < argc; i++) {
		if (strlen(argv[i]) != optlen)
			continue;
		if (memcmp(argv[i], opt, optlen) == 0)
			return true;
	}
	return false;
}

