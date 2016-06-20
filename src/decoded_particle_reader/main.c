/* read decoded particle files and print position of particles */

#include <stdio.h>
#include <stdlib.h>

#include "GetOption/GetOption.h"
#include "Gadget/BufVector3D.h"

int
main(int argc, char *argv[])
{
	/* sequential index of file names in argv[] */
	int beg = -1;
	int end = -1;
	
	BufFloatVector3D *bfv;
	int i;
	
	getOptionValueRangeFromArgs(argc, argv, &beg, &end, "-f");
	if (beg < 0) {
		fprintf(stderr, "Usage: %s -f decoded_particle_files(more than or equal to 1 files)\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	puts("--- files to read ---");
	for (i = beg; i < end; i++)
		puts(argv[i]);
	
	bfv = constructBufFloatVector3D();
	for (i = beg; i < end; i++) {
		unsigned long long n;
		
		bfv = readDecodedParticles(bfv, argv[i]);
		for (n = 0; n < bfv->nfvs; n++) {
			float *fv = getVectorBufFloatVector3D(bfv, n);
			printf("{%f %f %f}\n", fv[0], fv[1], fv[2]);
		}
		resetBufFloatVector3D(bfv);
	}
	destructBufFloatVector3D(bfv);
	
	return 0;
}

