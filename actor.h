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

extern obj_t *objlet[52];    // 52 pointers to objects, a-z & A-Z

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
        int phy;
        int intl;
        int wis;
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

typedef struct actorstruct {            // actor_t 
        short        id;                // id = monsterdef id
        unsigned int mid;               // mid = unique id for this monster; in monsterdefs mid = index into aitable! (that should work right?)
        short        x, y, oldx, oldy, px, py;
        short        viewradius;
        char         name[50];
        int          hp, maxhp;
        int          xp;
        short        ac;
        uattr_t      attr;
        int          level;
        short        race, cla;
        obj_t        *weapon;           // currently wielded weapon
        obj_t        *inventory;
        wear_t       w;
        long         flags;
        int          c;                 // character, for monsters.
        double       speed;
        double       movement;
        long long    ticks;
        int          thac0;
        float        skill[MAX_SKILLS];
        //char         wvfactor;
        //short        worldview;

        /* monster specific stuff */
        void               (*ai)(struct actorstruct *);      // artificial intelligence handler!!
        short              goalx, goaly;                      // for simple outdoor pathfinder ai
        struct actorstruct *prev;
        struct actorstruct *next;
        struct actorstruct *attacker;
        struct actorstruct *head;
} actor_t;

char   get_first_free_letter();
char   slot_to_letter(int i);
int    letter_to_slot(char c);
obj_t *get_object_from_letter(char c);
int object_to_slot(obj_t *o);
void assign_slot(obj_t *o);
void unassign_object(obj_t *o);


bool actor_in_lineofsight(actor_t *src, actor_t *dest);
bool next_to(actor_t *a, actor_t *b);
void attack(actor_t *attacker, actor_t *victim);

#endif
