
/*

 Written by: Paul E. Martz
 
 Copyright 1997 by Paul E. Martz, all right reserved

 Non-commercial use by individuals is permitted.

*/

#ifdef __cplusplus
extern "C" {
#endif


#ifdef DEBUG
void dump1DFractArray (float *, int);
void dump2DFractArray (float *, int);
#endif /* DEBUG */

void fill1DFractArray (float *, int, int, float, float);
void fill2DFractArray (float *, int, int, float, float);

float *alloc1DFractArray (int);
float *alloc2DFractArray (int);

void freeFractArray (float *);


void draw1DFractArrayAsLines (float *, int);
void draw2DFractArrayAsLines (float *, int);
void draw2DFractArrayAsTriangles (float *, int);


#ifdef __cplusplus
}
#endif
