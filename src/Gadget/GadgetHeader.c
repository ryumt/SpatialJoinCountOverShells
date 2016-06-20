#include <stdio.h>
#include <math.h>

#include "Gadget/GadgetHeader.h"


inline 
void 
initGadgetDecodeParams(GadgetDecodeParams *gdp, GadgetHeader *ghdr, int ntile)
{	
	/* total number of particles */
	gdp->nparticles_total = (long long int)ghdr->npartTotal[1] + ((long long int)ghdr->npartTotal[2] << 32);
	/* number of particles at 1-dimension, 2048 */
        gdp->nparticles_at1d = pow((double)gdp->nparticles_total + 0.5, 1/3.);
        /* average interval between cells */
        gdp->cell_interval = ghdr->BoxSize / (double)gdp->nparticles_at1d;
	/* number of particles per tile */
	/* タイリングした時のユニットとなるdummy_glassファイルの1Dあたりの粒子数。ntile = 1の場合は単にN1Dと同一 */
        gdp->nparticles_partile = gdp->nparticles_at1d / ntile;
	/* unit separation for displacement */
	gdp->unitsep = (ghdr->disp_max - ghdr->disp_min) / pow(2, ghdr->pos_bits);
	/* set id offset */
	gdp->id_start = ghdr->id_start;
}

inline 
void 
printGadgetHeader(GadgetHeader *ghdr)
{
	printf("npart={%d, %d, %d, %d, %d, %d}\n"
	       "mass={%f, %f, %f, %f, %f, %f}\n"
	       "time=%f, redshift=%f\n"
	       "flag_sfr=%d, flag_feedback=%d\n"
	       "npartTotal={%d, %d, %d, %d, %d, %d}\n"
	       "flag_cooling=%d, num_files=%d\n"
	       "BoxSize=%f, Omega0=%f, OmegaLambda=%f, HubbleParam=%f\n"
	       "flag_stellarge=%d, flag_metals=%d, hashsize=%d\n"
	       "disp_min=%f, disp_max=%f, pos_bits=%d\n"
	       "id_start=%lld, id_end=%lld\n"
	       "fill=%s\n", 
	       ghdr->npart[0], ghdr->npart[1], ghdr->npart[2], ghdr->npart[3], ghdr->npart[4], ghdr->npart[5], 
	       ghdr->mass[0], ghdr->mass[1], ghdr->mass[2], ghdr->mass[3], ghdr->mass[4], ghdr->mass[5], 
	       ghdr->time, ghdr->redshift, 
	       ghdr->flag_sfr, ghdr->flag_feedback, 
	       ghdr->npartTotal[0], ghdr->npartTotal[1], ghdr->npartTotal[2],
	       ghdr->npartTotal[3], ghdr->npartTotal[4], ghdr->npartTotal[5], 
	       ghdr->flag_cooling, ghdr->num_files, 
	       ghdr->BoxSize, ghdr->Omega0, ghdr->OmegaLambda, ghdr->HubbleParam, 
	       ghdr->flag_stellarage, ghdr->flag_metals, ghdr->hashsize, 
	       ghdr->disp_min, ghdr->disp_max, ghdr->pos_bits, 
	       ghdr->id_start, ghdr->id_end, ghdr->fill
		);
}

