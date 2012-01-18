/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef _GT_UTILS_H
#define _GT_UTILS_H

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

int isarmor(obj_t *o);
int dice(int num, int sides, signed int modifier);
int perc(int i);
int ri(int a, int b);

extern int garbageindex;
extern void *garbage[10000];
#endif
