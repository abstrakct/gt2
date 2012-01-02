/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Copyright 2011 Rolf Klausen
 */

#ifndef _MONSTERS_H
#define _MONSTERS_H


//typedef struct monster monster_t;
typedef actor_t monster_t;
typedef void (*aifunction)(monster_t *);

extern aifunction aitable[];

#define MF_ISHOSTILE         0x00000001
#define MF_CANUSEWEAPON      0x00000002
#define MF_CANUSEARMOR       0x00000004
#define MF_CANHAVEGOLD       0x00000008
#define MF_CANUSESIMPLESWORD 0x00000010

// Prototypes
monster_t get_monsterdef(int n);
void spawn_monster(int n, monster_t *head);
bool spawn_monster_at(int x, int y, int n, monster_t *head, void *level);

// AI
void simpleai(monster_t *m);
void advancedai(monster_t *m);

#endif
