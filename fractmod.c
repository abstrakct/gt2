
/*

 Written by: Paul E. Martz
 
 Copyright 1997 by Paul E. Martz, all right reserved

 Non-commercial use by individuals is permitted.

*/

#include <stdio.h>
#include <stdlib.h>
#include "math.h"

#include "fractmod.h"


/* Compiling with DEBUG defined can produce enormous amounts of output
   to stdout. Beware.
#define DEBUG
   */

#ifdef _WINDOWS
/* Was written for HPUX. Handle Win32 compiles */
#define random() rand()
#define srandom(x) srand(x)
#endif

/*
 * randNum - Return a random floating point number such that
 *      (min <= return-value <= max)
 * 32,767 values are possible for any given range.
 */
static float randnum (float min, float max)
{
	int r;
    float	x;
    
	r = random ();
    x = (float)(r & 0x7fff) /
		(float)0x7fff;
    return (x * (max - min) + min);
} 


/*
 * fractRand is a useful interface to randnum.
 */
#define fractRand(v) randnum (-v, v)



/*
 * avgEndpoints - Given the i location and a stride to the data
 * values, return the average those data values. "i" can be thought of
 * as the data value in the center of two line endpoints. We use
 * "stride" to get the values of the endpoints. Averaging them yields
 * the midpoint of the line.
 *
 * Called by fill1DFractArray.
 */
static float avgEndpoints (int i, int stride, float *fa)
{
    return ((float) (fa[i-stride] +
		     fa[i+stride]) * .5f);
}

/*
 * avgDiamondVals - Given the i,j location as the center of a diamond,
 * average the data values at the four corners of the diamond and
 * return it. "Stride" represents the distance from the diamond center
 * to a diamond corner.
 *
 * Called by fill2DFractArray.
 */
static float avgDiamondVals (int i, int j, int stride,
			     int size, int subSize, float *fa)
{
    /* In this diagram, our input stride is 1, the i,j location is
       indicated by "X", and the four value we want to average are
       "*"s:
           .   *   .

           *   X   *

           .   *   .
       */

    /* In order to support tiled surfaces which meet seamless at the
       edges (that is, they "wrap"), We need to be careful how we
       calculate averages when the i,j diamond center lies on an edge
       of the array. The first four 'if' clauses handle these
       cases. The final 'else' clause handles the general case (in
       which i,j is not on an edge).
     */
    if (i == 0)
	return ((float) (fa[(i*size) + j-stride] +
			 fa[(i*size) + j+stride] +
			 fa[((subSize-stride)*size) + j] +
			 fa[((i+stride)*size) + j]) * .25f);
    else if (i == size-1)
	return ((float) (fa[(i*size) + j-stride] +
			 fa[(i*size) + j+stride] +
			 fa[((i-stride)*size) + j] +
			 fa[((0+stride)*size) + j]) * .25f);
    else if (j == 0)
	return ((float) (fa[((i-stride)*size) + j] +
			 fa[((i+stride)*size) + j] +
			 fa[(i*size) + j+stride] +
			 fa[(i*size) + subSize-stride]) * .25f);
    else if (j == size-1)
	return ((float) (fa[((i-stride)*size) + j] +
			 fa[((i+stride)*size) + j] +
			 fa[(i*size) + j-stride] +
			 fa[(i*size) + 0+stride]) * .25f);
    else
	return ((float) (fa[((i-stride)*size) + j] +
			 fa[((i+stride)*size) + j] +
			 fa[(i*size) + j-stride] +
			 fa[(i*size) + j+stride]) * .25f);
}


/*
 * avgSquareVals - Given the i,j location as the center of a square,
 * average the data values at the four corners of the square and return
 * it. "Stride" represents half the length of one side of the square.
 *
 * Called by fill2DFractArray.
 */
static float avgSquareVals (int i, int j, int stride, int size, float *fa)
{
    /* In this diagram, our input stride is 1, the i,j location is
       indicated by "*", and the four value we want to average are
       "X"s:
           X   .   X

           .   *   .

           X   .   X
       */
    return ((float) (fa[((i-stride)*size) + j-stride] +
		     fa[((i-stride)*size) + j+stride] +
		     fa[((i+stride)*size) + j-stride] +
		     fa[((i+stride)*size) + j+stride]) * .25f);
}


#ifdef DEBUG
/*
 * dump1DFractArray - Use for debugging.
 */
void dump1DFractArray (float *fa, int size)
{
    int	i;

    for (i=0; i<size; i++)
	printf ("(%.2f)   ", fa[i]);
    printf ("\n");
}
#endif

/*
 * dump2DFractArray - Use for debugging.
 */
void dump2DFractArray (float *fa, int size)
{
    int	i, j;

    for (i=0; i<size; i++) {
	j=0;
	printf ("[%d,%d]: ", i, j);
	for (; j<size; j++) {
	    printf ("(%.2f)   ",
		    fa[(i*size)+j]);
	}
	printf ("\n");
    }
}


/*
 * powerOf2 - Returns 1 if size is a power of 2. Returns 0 if size is
 * not a power of 2, or is zero.
 */
static int powerOf2 (int size)
{
    int i, bitcount = 0;

    /* Note this code assumes that (sizeof(int)*8) will yield the
       number of bits in an int. Should be portable to most
       platforms. */
    for (i=0; i<sizeof(int)*8; i++)
	if (size & (1<<i))
	    bitcount++;
    if (bitcount == 1)
	/* One bit. Must be a power of 2. */
	return (1);
    else
	/* either size==0, or size not a power of 2. Sorry, Charlie. */
	return (0);
}


/*
 * fill1DFractArray - Tessalate an array of values into an
 * approximation of fractal Brownian motion.
 */
void fill1DFractArray (float *fa, int size,
		       int seedValue, float heightScale, float h)
{
    int	i;
    int	stride;
    int subSize;
	float ratio, scale;

    if (!powerOf2(size) || (size==1)) {
	/* We can't tesselate the array if it is not a power of 2. */
#ifdef DEBUG
	printf ("Error: fill1DFractArray: size %d is not a power of 2.\n");
#endif /* DEBUG */
	return;
    }

    /* subSize is the dimension of the array in terms of connected line
       segments, while size is the dimension in terms of number of
       vertices. */
    subSize = size;
    size++;
    
    /* initialize random number generator */
    srandom (seedValue);

#ifdef DEBUG
    printf ("initialized\n");
    dump1DFractArray (fa, size);
#endif

	/* Set up our roughness constants.
	   Random numbers are always generated in the range 0.0 to 1.0.
	   'scale' is multiplied by the randum number.
	   'ratio' is multiplied by 'scale' after each iteration
	   to effectively reduce the randum number range.
	   */
	ratio = (float) pow (2.,-h);
	scale = heightScale * ratio;

    /* Seed the endpoints of the array. To enable seamless wrapping,
       the endpoints need to be the same point. */
    stride = subSize / 2;
    fa[0] =
      fa[subSize] = 0.f;

#ifdef DEBUG
    printf ("seeded\n");
    dump1DFractArray (fa, size);
#endif

    while (stride) {
		for (i=stride; i<subSize; i+=stride) {
			fa[i] = scale * fractRand (.5f) +
				avgEndpoints (i, stride, fa);

			/* reduce random number range */
			scale *= ratio;

			i+=stride;
		}
		stride >>= 1;
    }

#ifdef DEBUG
    printf ("complete\n");
    dump1DFractArray (fa, size);
#endif
}


/*
 * fill2DFractArray - Use the diamond-square algorithm to tessalate a
 * grid of float values into a fractal height map.
 */
void fill2DFractArray (float *fa, int size,
		       int seedValue, float heightScale, float h)
{
    int	i, j;
    int	stride;
    int	oddline;
    int subSize;
	float ratio, scale;

    if (!powerOf2(size) || (size==1)) {
	/* We can't tesselate the array if it is not a power of 2. */
#ifdef DEBUG
	printf ("Error: fill2DFractArray: size %d is not a power of 2.\n");
#endif /* DEBUG */
	return;
    }

    /* subSize is the dimension of the array in terms of connected line
       segments, while size is the dimension in terms of number of
       vertices. */
    subSize = size;
    size++;
    
    /* initialize random number generator */
    srandom (seedValue);
    
#ifdef DEBUG
    printf ("initialized\n");
    dump2DFractArray (fa, size);
#endif

	/* Set up our roughness constants.
	   Random numbers are always generated in the range 0.0 to 1.0.
	   'scale' is multiplied by the randum number.
	   'ratio' is multiplied by 'scale' after each iteration
	   to effectively reduce the randum number range.
	   */
	ratio = (float) pow (2.,-h);
	scale = heightScale * ratio;

    /* Seed the first four values. For example, in a 4x4 array, we
       would initialize the data points indicated by '*':

           *   .   .   .   *

           .   .   .   .   .

           .   .   .   .   .

           .   .   .   .   .

           *   .   .   .   *

       In terms of the "diamond-square" algorithm, this gives us
       "squares".

       We want the four corners of the array to have the same
       point. This will allow us to tile the arrays next to each other
       such that they join seemlessly. */

    stride = subSize / 2;
    fa[(0*size)+0] =
      fa[(subSize*size)+0] =
        fa[(subSize*size)+subSize] =
          fa[(0*size)+subSize] = 0.f;
    
#ifdef DEBUG
    printf ("seeded\n");
    dump2DFractArray (fa, size);
#endif

    /* Now we add ever-increasing detail based on the "diamond" seeded
       values. We loop over stride, which gets cut in half at the
       bottom of the loop. Since it's an int, eventually division by 2
       will produce a zero result, terminating the loop. */
    while (stride) {
		/* Take the existing "square" data and produce "diamond"
		   data. On the first pass through with a 4x4 matrix, the
		   existing data is shown as "X"s, and we need to generate the
	       "*" now:

               X   .   .   .   X

               .   .   .   .   .

               .   .   *   .   .

               .   .   .   .   .

               X   .   .   .   X

	      It doesn't look like diamonds. What it actually is, for the
	      first pass, is the corners of four diamonds meeting at the
	      center of the array. */
		for (i=stride; i<subSize; i+=stride) {
			for (j=stride; j<subSize; j+=stride) {
				fa[(i * size) + j] =
					scale * fractRand (.5f) +
					avgSquareVals (i, j, stride, size, fa);
				j += stride;
			}
			i += stride;
		}
#ifdef DEBUG
		printf ("Diamonds:\n");
		dump2DFractArray (fa, size);
#endif

		/* Take the existing "diamond" data and make it into
	       "squares". Back to our 4X4 example: The first time we
	       encounter this code, the existing values are represented by
	       "X"s, and the values we want to generate here are "*"s:

               X   .   *   .   X

               .   .   .   .   .

               *   .   X   .   *

               .   .   .   .   .

               X   .   *   .   X

	       i and j represent our (x,y) position in the array. The
	       first value we want to generate is at (i=2,j=0), and we use
	       "oddline" and "stride" to increment j to the desired value.
	       */
		oddline = 0;
		for (i=0; i<subSize; i+=stride) {
		    oddline = (oddline == 0);
			for (j=0; j<subSize; j+=stride) {
				if ((oddline) && !j) j+=stride;

				/* i and j are setup. Call avgDiamondVals with the
				   current position. It will return the average of the
				   surrounding diamond data points. */
				fa[(i * size) + j] =
					scale * fractRand (.5f) +
					avgDiamondVals (i, j, stride, size, subSize, fa);

				/* To wrap edges seamlessly, copy edge values around
				   to other side of array */
				if (i==0)
					fa[(subSize*size) + j] =
						fa[(i * size) + j];
				if (j==0)
					fa[(i*size) + subSize] =
						fa[(i * size) + j];

				j+=stride;
			}
		}
#ifdef DEBUG
		printf ("Squares:\n");
		dump2DFractArray (fa, size);
#endif

		/* reduce random number range. */
		scale *= ratio;
		stride >>= 1;
    }

#ifdef DEBUG
    printf ("complete\n");
    dump2DFractArray (fa, size);
#endif
}


/*
 * alloc1DFractArray - Allocate float-sized data points for a 1D strip
 * containing size line segments.
 */
float *alloc1DFractArray (int size)
{
    /* Increment size (see comment in alloc2DFractArray, below, for an
       explanation). */
    size++;

    return ((float *) malloc (sizeof(float) * size));
}

/*
 * alloc2DFractArray - Allocate float-sized data points for a sizeXsize
 * mesh.
 */
float *alloc2DFractArray (int size)
{
    /* For a sizeXsize array, we need (size+1)X(size+1) space. For
       example, a 2x2 mesh needs 3x3=9 data points: 

           *   *   *

           *   *   *

           *   *   *

       To account for this, increment 'size'. */
    size++;

    return ((float *) malloc (sizeof(float) * size * size));
}

/*
 * freeFractArray - Takes a pointer to float and frees it. Can be used
 * to free data that was allocated by either alloc1DFractArray or
 * alloc2DFractArray.
 */
void freeFractArray (float *fa)
{
    free (fa);
}

/*
extern void draw2DLine (float, float, float, float);
*/
/*
 * draw1DFractArrayAsLines - Draws the height map as a single set of
 * connected line segments.
 *
 * This is a simplified routine intended for getting things up and
 * running quickly, and as a demonstration of how to walk through the
 * array.
 *
 * To use this routine, you MUST define your own "draw2DLine" function
 * according to the extern definition below. It takes 4 parameters,
 * the X and Y world coordinates of the first endpoint followed by the
 * X and Y world coordinates of the second endpoint.
 *
 * The X coordinates will be distributed evenly from -1.0 to
 * 1.0. Corresponding Y coordinates will be extracted from the fract
 * array.
 */
/*
void draw1DFractArrayAsLines (float *fa, int size)
{
    int	i;
    float	x, inc;

    inc = 2.f / size;
    x = -1.f;
    for (i=0; i<size; i++) {
	draw2DLine (x, fa[i],
		    x+inc, fa[i+1]);
	x += inc;
    }
}
*/
/*
 * draw2DFractArrayAsLines - Draws the height map as a set of line
 * segments comprising a grid.
 *
 * This is a simplified routine intended for getting things up and
 * running quickly, and as a demonstration of how to walk through the
 * array.
 *
 * To use this routine, you MUST define your own "draw3DLine" function
 * according to the extern definition below. It takes 6 parameters,
 * the X, Y, and Z world coordinates of the first endpoint followed by
 * the X, Y, and Z world coordinates of the second endpoint.
 *
 * X and Z coordinates will be distributed evenly over a grid from
 * -1.0 to 1.0 along both axes. Corresponding Y coordinates will be
 * extracted from the fract array.
 */
/*
void draw2DFractArrayAsLines (float *fa, int size)
{
    extern void draw3DLine (float, float, float, float, float, float);
    int	i, j;
    float	x, z, inc;
    const int	subSize = size;

    size++;
    inc = 2.f / subSize;
    z = -1.f;
    for (i=0; i<size; i++) {
	x = -1.f;
	for (j=0; j<subSize; j++) {
	    draw3DLine (x, fa[(i*size)+j], z,
			x+inc, fa[(i*size+j+1)], z);
	    if (i<subSize)
		draw3DLine (x, fa[(i*size)+j], z,
			    x, fa[((i+1)*size+j)], z+inc);
	    x += inc;
	}
	if (i<subSize)
	    draw3DLine (x, fa[(i*size)+j], z,
			x, fa[((i+1)*size+j)], z+inc);
	z += inc;
    }
}
*/

static void genNormal (float x1, float y1, float z1,
		       float x2, float y2, float z2,
		       float x3, float y3, float z3,
		       float *normal)
{
    float	len;
    float	v1x, v1y, v1z;
    float	v2x, v2y, v2z;


    v1x = x2 - x1;
    v1y = y2 - y1;
    v1z = z2 - z1;
    v2x = x3 - x1;
    v2y = y3 - y1;
    v2z = z3 - z1;

    normal[0] = v1y*v2z - v1z*v2y;
    normal[1] = v1z*v2x - v1x*v2z;
    normal[2] = v1x*v2y - v1y*v2x;

    len = (float) sqrt (normal[0]*normal[0] + normal[1]*normal[1] +
			normal[2]*normal[2]);

    normal[0] /= len;
    normal[1] /= len;
    normal[2] /= len;
}

/*
 * draw2DFractArrayAsTriangles - Draws the height map as a set of
 * triangular facets with a corresponding facet normal.
 *
 * This is a simplified routine intended for getting things up and
 * running quickly, and as a demonstration of how to walk through the
 * array.
 *
 * To use this routine, you MUST define your own "draw3DTriangle"
 * function according to the extern definition below. It takes 12
 * (ugh!) parameters: the first 9 are the X, Y, and Z world
 * coordinates of the three vertices, and the last three parameters
 * are the X, Y, and Z cartesian components of the unit-length facet
 * normal.
 *
 * X and Z coordinates will be distributed evenly over a grid from
 * -1.0 to 1.0 along both axes. Corresponding Y coordinates will be
 * extracted from the fract array. Normals will be gererated favoring
 * the "UP" or positive Y direction, and vertex order will use the
 * right hand rule to match the normal. ("Right hand rule": When
 * viewed from the direction the normal is pointing, the vertex
 * ordering will be counter-clockwise.)
 */
/*
void draw2DFractArrayAsTriangles (float *fa, int size)
{
    extern void draw3DTriangle (float, float, float, float, float, float,
				float, float, float, float, float, float);
    int	i, j;
    float	x, z, inc;
    const int	subSize = size;

    size++;
    inc = 2.f / subSize;
    z = -1.f;
    for (i=0; i<subSize; i++) {
	x = -1.f;
	for (j=0; j<subSize; j++) {
	    float	normal[3];

	    genNormal (x, fa[(i*size)+j], z,
		       x, fa[((i+1)*size)+j], z+inc,
		       x+inc, fa[(i*size+j+1)], z,
		       normal);
	    draw3DTriangle (x, fa[(i*size)+j], z,
			    x, fa[((i+1)*size)+j], z+inc,
			    x+inc, fa[(i*size+j+1)], z,
			    normal[0], normal[1], normal[2]);

	    genNormal (x, fa[((i+1)*size)+j], z+inc,
		       x+inc, fa[((i+1)*size)+j+1], z+inc,
		       x+inc, fa[(i*size+j+1)], z,
		       normal);
	    draw3DTriangle (x, fa[((i+1)*size)+j], z+inc,
			    x+inc, fa[((i+1)*size)+j+1], z+inc,
			    x+inc, fa[(i*size+j+1)], z,
			    normal[0], normal[1], normal[2]);

	    x += inc;
	}
	z += inc;
    }
}
*/
/*
void main()
{
        float *f;

        f = alloc2DFractArray(128);
        fill2DFractArray(f, 128, 577, 1.0, 0.9);
        dump2DFractArray(f, 128);

        return;
}*/
