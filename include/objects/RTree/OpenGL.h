#ifndef RTREE_OPENGL_HEAD__
#define RTREE_OPENGL_HEAD__

#include "types/ulong.h"
#include "objects/RTree/RTree.h"

/*** public ***/
extern int drawRTree(RTree *rt);

/*** private ***/
#ifdef RTREE_OPENGL_SOURCE__
static void DrawPoints(float *p);
static void DrawMbr(Mbr *mbr, int level);
static void DrawMbrs(RTree *rt);
static void DrawMbrsRecursive(RTreeNode *node, int level);

static void Display(void);
static void Resize(int w, int h);
static void Mouse(int button, int state, int x, int y); 
static void MouseMotionLeft(int x, int y);
static void MouseMotionRight(int x, int y);
static void Keyboard(unsigned char key, int x, int y);
static void Idle(void);

static void Reset(void);
static void Init(RTree *rt);
#endif //RTREE_OPENGL_SOURCE__

#endif //RTREE_OPENGL_HEAD__
