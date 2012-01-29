/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef _GT_UTILS_H
#define _GT_UTILS_H

#define clearbit(a, b) ((a) &= ~(b))
#define setbit(a, b)   ((a) |=  (b))
#define hasbit(a, b)   ((a) &   (b))

#define MAX_GARBAGE 100000
// d(a, b)
// "throw" a b-sided dices without any modifiers
#define d(a,b) dice(a,b,0)

// ri(a, b)
// pick a random number in the range [a, b]
//#define ri(a,b) (a + (rand() % (b-a+1)))

void get_version_string(char *s);
void die(char *m, ...);

void *gtmalloc(size_t size);
void *gtcalloc(size_t num, size_t size);
void gtfree(void *ptr);

int dice(int num, int sides, signed int modifier);
int perc(int i);
int ri(int a, int b);

void you(char *fmt, ...);
void youc(int color, char *fmt, ...);
void yousee(char *fmt, ...);
void gtprintf(char *fmt, ...);
void gtprintfc(int color, char *fmt, ...);
char *a_an(char *s);

extern int garbageindex;
extern void *garbage[MAX_GARBAGE];
#endif
