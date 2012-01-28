/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * actors (player, monsters)
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef _ACTOR_H
#define _ACTOR_H

#define MAX_SKILLS 10

typedef struct { // sattr_t
        signed char str;
        signed char phys;
        signed char intl;
        signed char know;
        signed char dex;
        signed char cha;
} sattr_t;

typedef struct { // uattr_t
        int str;
        int phys;
        int intl;
        int know;
        int dex;
        int cha;
} uattr_t;

typedef struct { // wear_t
        struct object *head;
        struct object *body;
        struct object *gloves;
        struct object *footwear;
        struct object *robe;
        struct object *amulet;
        struct object *leftring;
        struct object *rightring;
} wear_t;

typedef struct actorstruct {     // actor_t
        short id;                          // id = monsterdef id
        unsigned int mid;                  // mid = unique id for this monster; in monsterdefs mid = index into aitable! (that should work right?)
        short x, y, oldx, oldy, px, py;
        short viewradius;
        char name[50];
        int hp, maxhp;
        int xp;
        short ac;
        uattr_t attr;
        int level;
        short race, cla;
        //char wvfactor;
        //short worldview;
        //struct object *weapon;
        obj_t *inventory;
        wear_t w;
        long flags;
        int c;             // character, for monsters.
        double speed;
        double movement;
        long long ticks;
        int thac0;
        float skill[MAX_SKILLS];
        /* monster specific stuff */
        void (*ai)(struct actorstruct *);      // artificial intelligence handler!!
        short goalx, goaly;                      // for simple outdoor pathfinder ai
        struct actorstruct *prev;
        struct actorstruct *next;
        struct actorstruct *attacker;
        struct actorstruct *head;
        // infinite recursion inclusion fuck shit level_t *l;                            // pointer to the level where it's at
} actor_t;

bool actor_in_lineofsight(actor_t *src, actor_t *dest);
bool next_to(actor_t *a, actor_t *b);
void attack(actor_t *attacker, actor_t *victim);

#endif
