/* decode compressed particle files and generate decoded particle files */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "GetOption/GetOption.h"
#include "Gadget/BufVector3D.h"


int
main(int argc, char *argv[])
{
	/* number of threads */
	int np = 1;
	/* sequential index of file names in argv[] */
	int beg = -1;
	int end = -1;
	
	BufUshortVector3D *buv;
	int i;
	
	
	getOptionValueFromArgs(argc, argv, &np, VALUE_TYPE_INT, "-np", 0);
	getOptionValueRangeFromArgs(argc, argv, &beg, &end, "-f");
	if (beg < 0) {
		fprintf(stderr, "Usage: %s -f particle_files_to_decode(more than or equal to 1 files)"
			"[-np number_of_threads]\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	printf("# of threads = %d\n", np);
	puts("--- files to decode ---");
	for (i = beg; i < end; i++)
		puts(argv[i]);
	
	buv = constructBufUshortVector3D();
	for (i = beg; i < end; i++)
		decodeAndOutputCompressedParticles(buv, argv[i], 1);
	destructBufUshortVector3D(buv);
	
	return 0;
}

