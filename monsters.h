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

extern unsigned int mid_counter;
extern aifunction aitable[];

#define MF_ISDEAD            (1 <<  1)
#define MF_ISHOSTILE         (1 <<  2)
#define MF_CANUSEWEAPON      (1 <<  3)
#define MF_CANUSEARMOR       (1 <<  4)
#define MF_CANHAVEGOLD       (1 <<  5)
#define MF_CANUSESIMPLESWORD (1 <<  6)
#define MF_SLEEPING          (1 <<  7)

// Prototypes
monster_t get_monsterdef(int n);
void spawn_monster(int n, monster_t *head, int maxlevel);
void spawn_monsters(int num, int max_level, void *p);
bool spawn_monster_at(int y, int x, int n, monster_t *head, void *level, int maxlevel);
void unspawn_monster(monster_t *m);
void kill_monster(void *level, monster_t *m, actor_t *killer);
void move_monsters();

// AI
void simpleai(monster_t *m);
void advancedai(monster_t *m);
void hostile_ai(monster_t *m);

void makedistancemap(int desty, int destx);
void makedistancemap(int desty, int destx);

#endif
