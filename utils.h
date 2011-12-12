/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef _GT_UTILS_H
#define _GT_UTILS_H

#define ri(a,b) (a + (rand() % b))
#define d(a,b) dice(a,b,0)

char *get_version_string();
void die(char *m);
void *gtmalloc(size_t size);
int isarmor(obj_t *o);
int dice(int num, int sides, signed int modifier);

#endif
