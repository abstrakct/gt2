/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * actors (player, monsters)
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef _ACTOR_H
#define _ACTOR_H

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
        int id;                  // e.g. for saving and loading etc.
        int x, y;
        int px, py;
        int viewradius;
        char name[50];
        int hp, maxhp;
        int xp;
        short ac;
        uattr_t attr;
        int level;
        short race, cla;
        //char wvfactor;
        //short worldview;
        //struct object *inventory;
        //struct object *weapon;
        wear_t w;
        long flags;
        int c;             // character, for monsters.
        double speed;
        double movement;
        void (*ai)(struct actorstruct *);      // artificial intelligence handler!!
        int goalx, goaly;                   // for simple outdoor pathfinder ai
        struct actorstruct *prev;
        struct actorstruct *next;
        struct actorstruct *attacker;
        struct actorstruct *head;
        int thac0;
        float skill[10];
} actor_t;

#endif
