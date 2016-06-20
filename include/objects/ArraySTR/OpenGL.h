#ifndef ARRAYSTR_OPENGL_HEAD__
#define ARRAYSTR_OPENGL_HEAD__

#include "types/ulong.h"
#include "objects/ArraySTR/ArraySTR.h"

/*** public ***/
extern int drawArraySTR(ArraySTR *art);

/*** private ***/
#ifdef ARRAYSTR_OPENGL_SOURCE__
static void DrawPoints(ArraySTR *art);
static void DrawMbr(Mbr *mbr, int level);
static void DrawMbrs(ArraySTR *art);
static void DrawMbrsRecursive(ArraySTR *art, ulong_t inode, int level);

static void Display(void);
static void Resize(int w, int h);
static void Mouse(int button, int state, int x, int y); 
static void MouseMotionLeft(int x, int y);
static void MouseMotionRight(int x, int y);
static void Keyboard(unsigned char key, int x, int y);
static void Idle(void);

static void Reset(void);
static void Init(ArraySTR *art);
#endif //ARRAYSTR_OPENGL_SOURCE__

#endif //ARRAYSTR_OPENGL_HEAD__
