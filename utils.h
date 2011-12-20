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
#define ri(a,b) (a + (rand() % (b-a+1)))

char *get_version_string();
void die(char *m);
void *gtmalloc(size_t size);
void *gtcalloc(size_t num, size_t size);
int isarmor(obj_t *o);
int dice(int num, int sides, signed int modifier);
int perc(int i);

#endif
