/* print halo data: {id, mass, position[3]} in specified files */

#include <stdio.h>
#include <stdlib.h>

#include "GetOption/GetOption.h"
#include "Gadget/BufHalo.h"

int
main(int argc, char *argv[])
{
	/* sequential index of file names in argv[] */
	int beg = -1;
	int end = -1;

	BufHalo *bh;
	int i;
	
	getOptionValueRangeFromArgs(argc, argv, &beg, &end, "-f");
	if (beg < 0) {
		fprintf(stderr, "Usage: %s -f halo_files(more than or equal to 1 files)\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	puts("--- files to read ---");
	for (i = beg; i < end; i++)
		puts(argv[i]);
	
	bh = constructBufHalo();
	for (i = beg; i < end; i++) {
		unsigned long long n;
		
		bh = readHalos(bh, argv[i]);
		for (n = 0; n < bh->nhalos; n++) {
			Halo *halo = getHaloBufHalo(bh, n);
			printf("%lu: %f {%f %f %f}\n", halo->id, halo->mass,
			       halo->point[0], halo->point[1], halo->point[2]);
		}
		resetBufHalo(bh);
	}
	destructBufHalo(bh);
	
	return 0;
}

