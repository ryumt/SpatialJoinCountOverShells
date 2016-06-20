#ifndef GADGETHEADER_HEAD__
#define GADGETHEADER_HEAD__

typedef struct GadgetHeader {
	int npart[6];
	double mass[6];
	double time;
	double redshift;
	int flag_sfr;
	int flag_feedback;
	int npartTotal[6];
	int flag_cooling;
	int num_files;
	double BoxSize;
	double Omega0;
	double OmegaLambda;
	double HubbleParam;
	int flag_stellarage;
	int flag_metals;
	int hashsize;
	float disp_min;
	float disp_max;
	int pos_bits;
	long long int id_start;
	long long int id_end;
	char fill[56];
} GadgetHeader;

typedef struct GadgetDecodeParams {
	long long int nparticles_total;
        int nparticles_at1d;
        float cell_interval;
        int nparticles_partile;
	double unitsep;
	long long int id_start;
} GadgetDecodeParams;


extern void initGadgetDecodeParams(GadgetDecodeParams *gdp, GadgetHeader *ghdr, int ntile);
extern void printGadgetHeader(GadgetHeader *ghdr);

#endif //GADGETHEADER_HEAD__
