/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef GT_UTILS_H
#define GT_UTILS_H

char *get_version_string();
void die(char *m);
void *gtmalloc(size_t size);
int isarmor(obj_t *o);

#endif
