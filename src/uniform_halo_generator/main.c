/* generate random positioned halos and write them into specified file. */
/* random value generating method is Mersenne Twister. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mt19937ar/mt19937ar.h"
#include "Gadget/Bound.h"
#include "Gadget/BufHalo.h"
#include "GetOption/GetOption.h"

static unsigned long Seed = 1659533;

int
main(int argc, char *argv[])
{
	unsigned long long int nparticles = 10000000;
	/* sequential index of file names in argv[] */
	int beg = -1;
	int end = -1;
	int i;
	
	getOptionValueFromArgs(argc, argv, &nparticles, VALUE_TYPE_ULONG, "-n", 0);
	getOptionValueFromArgs(argc, argv, &Seed, VALUE_TYPE_ULONG, "-s", 0);
	getOptionValueRangeFromArgs(argc, argv, &beg, &end, "-f");
	if (beg < 0) {
		fprintf(stderr, "Usage: %s -f output_halo_file_name(more than or equal to 1 files) "
			"[-n #_of_halos_in_each_file] [-s seed_for_random]\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	printf("seed for random position = %lu\n", Seed);
	printf("# of halos in each file = %llu\n", nparticles);
	puts("--- files to generate ---");
	for (i = beg; i < end; i++)
		puts(argv[i]);
	
	init_genrand(Seed);
	for (i = beg; i < end; i++) {
		int axis;
		FILE *fp;
		unsigned long long int n;
		
		fp = fopen(argv[i], "wb");
		if (fp == NULL) {
			fprintf(stderr, "ERROR: cannot open %s in %s()\n", argv[i], __func__);
			exit(EXIT_FAILURE);
		}
		/* write dummy header */
		for (n = 0; n < N_HEADER_LINES; n++)
			fprintf(fp, "# dummy header %llu\n", n);
		/* write halo data */
		for (n = 0; n < nparticles; n++) {
			static unsigned long long int id = 0;
			float mass;
			float pos[3];
			
			mass = genrand_real1() * (UPP_BOUND * UPP_BOUND) * (UPP_BOUND * UPP_BOUND);
			for (axis = 0; axis < 3; axis++)
				pos[axis] = genrand_real1() * (UPP_BOUND - LOW_BOUND) + LOW_BOUND;
			
			fprintf(fp, "%llu 0 %g 0 0 0 0 0 "
				"%f %f %f 0 0 0 0 0 "
				"0 0 0 0 0 0 0 0 "
				"0 0 0 0 0 0 0 0 "
				"0 0 0 0 0 0 0 0\n", 
				id++, mass, pos[0], pos[1], pos[2]);
			
			if (ferror(fp)) {
				char msg[FILENAME_MAX * 10];
				snprintf(msg, sizeof(msg), "ERROR: while writing %s in %s()\n", argv[i], __func__);
				perror(msg);
				fclose(fp);
				exit(EXIT_FAILURE);
			}
		}
		fclose(fp);
	}
	
	return 0;
}

