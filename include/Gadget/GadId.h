#ifndef GADID_HEAD__
#define GADID_HEAD__


typedef struct GadId {
	/* number of unit dummy_glass file tiled in 1 and 2D */
	int Nt1D;
	int Nt2D;
	
	/* number of particles in a dummy_glass file */
	long long int Ng1D;
	long long int Ng2D;
	long long int Ng3D;
} GadId;


/*** public ***/
extern GadId *constructGadId(const int a, const int b);
extern void destructGadId(GadId *gi);
extern void initGadId(GadId *gi, const int a, const int b);

extern void id2posGadId(GadId *gi, const long long int id, int *px, int *py, int *pz);
extern long long int pos2idGadId(GadId *gi, const int px, const int py, const int pz);


/*** private ***/
#ifdef GADID_SOURCE__
static void IDall2IDpartGadId(GadId *gi, const long long int ID, long long int *IDt, long long int *IDg);
static long long int IDpart2IDallGadId(GadId *gi, const long long int IDt, const long long int IDg);

static void IDg2IndexGadId(GadId *gi, const long long int IDg, int *a, int *b, int *c);
static long long int Index2IDgGadId(GadId *gi, const int a, const int b, const int c);

static void IDt2IndexGadId(GadId *gi, const long long int IDt, int *a, int *b, int *c);
static long long int Index2IDtGadId(GadId *gi, const int a, const int b, const int c);

static int Index2GridposGadId(GadId *gi, const int nt, const int ng);
static void Gridpos2IndexGadId(GadId *gi, const int a, int *nt, int *ng);
#endif //GADID_SOURCE__

#endif //GADID_HEAD__

