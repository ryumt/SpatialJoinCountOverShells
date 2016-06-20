#define GADID_SOURCE__

#include <stdlib.h>
#include "Gadget/GadId.h"


inline 
GadId *
constructGadId(const int a, const int b)
{
	GadId *gi;
	
	gi = (void *)malloc(sizeof(*gi));
	if (gi != NULL)
		initGadId(gi, a, b);
	return gi;
}

inline 
void 
destructGadId(GadId *gi)
{
	free(gi);
}

inline 
void 
initGadId(GadId *gi, const int a, const int b)
{
	gi->Nt1D = a;
	gi->Nt2D = a * a;
	gi->Ng1D = b;
	gi->Ng2D = b * b;
	gi->Ng3D = (long long int)b * (long long int)b * (long long int)b;
}


inline 
void 
id2posGadId(GadId *gi, const long long int id, int *px, int *py, int *pz)
{
	long long int IDt, IDg;
	int ig, jg, kg;
	int it, jt, kt;
	
	/* id -= ((id>>34)<<34); */
	IDall2IDpartGadId(gi, id, &IDt, &IDg);
	IDg2IndexGadId(gi, IDg, &ig, &jg, &kg);
	IDt2IndexGadId(gi, IDt, &it, &jt, &kt);
	
	*px = Index2GridposGadId(gi, it, ig);
	*py = Index2GridposGadId(gi, jt, jg);
	*pz = Index2GridposGadId(gi, kt, kg);
}

inline 
long long int 
pos2idGadId(GadId *gi, const int px, const int py, const int pz)
{
	long long int id;
	long long int IDt, IDg;
	int ig, jg, kg;
	int it, jt, kt;
	
	Gridpos2IndexGadId(gi, px, &it, &ig);
	Gridpos2IndexGadId(gi, py, &jt, &jg);
	Gridpos2IndexGadId(gi, pz, &kt, &kg);
	
	IDt = Index2IDtGadId(gi, it, jt, kt);
	IDg = Index2IDgGadId(gi, ig, jg, kg);
	id = IDpart2IDallGadId(gi, IDt, IDg);
	
	return id;
}


static inline 
void 
IDall2IDpartGadId(GadId *gi, const long long int ID, long long int *IDt, long long int *IDg)
{
	*IDt = (ID - 1) / gi->Ng3D;
	*IDg = ID - 1 - *IDt * gi->Ng3D;
}

static inline 
long long int 
IDpart2IDallGadId(GadId *gi, const long long int IDt, const long long int IDg)
{
	return IDg + 1 + IDt * gi->Ng3D;
}

static inline 
void 
IDg2IndexGadId(GadId *gi, const long long int IDg, int *a, int *b, int *c)
{
	*a = IDg / gi->Ng2D;
	*b = (IDg - *a * gi->Ng2D) / gi->Ng1D;
	*c = IDg - *a * gi->Ng2D - *b * gi->Ng1D;
}

static inline 
long long int 
Index2IDgGadId(GadId *gi, const int a, const int b, const int c)
{
	return a * gi->Ng2D + b * gi->Ng1D + c;
}

static inline 
void 
IDt2IndexGadId(GadId *gi, const long long int IDt, int *a, int *b, int *c)
{
	*a = IDt / gi->Nt2D;
	*b = (IDt - *a * gi->Nt2D) / gi->Nt1D;
	*c = IDt - *a * gi->Nt2D - *b * gi->Nt1D;
}

static inline 
long long int 
Index2IDtGadId(GadId *gi, const int a, const int b, const int c)
{
	return a * gi->Nt2D + b * gi->Nt1D + c;
}

static inline 
int 
Index2GridposGadId(GadId *gi, const int nt, const int ng)
{
	return nt * gi->Ng1D + ng;
}

static inline 
void 
Gridpos2IndexGadId(GadId *gi, const int a, int *nt, int *ng)
{
	*nt = a / gi->Ng1D;
	*ng = a - *nt * gi->Ng1D;
}

