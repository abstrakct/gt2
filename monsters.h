/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Copyright 2011 Rolf Klausen
 */

#ifndef _MONSTERS_H
#define _MONSTERS_H


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
        struct object *ring[10];
} wear_t;


struct monster {
        char name[50];
	int x, y;
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
        void (*ai)(struct monster *);      // artificial intelligence handler!!
        int goalx, goaly;                   // for simple outdoor pathfinder ai
        struct monster *prev;
        struct monster *next;
        struct monster *attacker;
        struct monster *head;
        int thac0;
        float skill[10];
};

typedef struct monster monster_t;
typedef void (*aifunction)(monster_t *);

extern aifunction aitable[];

#define MF_ISHOSTILE         0x00000001
#define MF_CANUSEWEAPON      0x00000002
#define MF_CANUSEARMOR       0x00000004
#define MF_CANHAVEGOLD       0x00000008
#define MF_CANUSESIMPLESWORD 0x00000010

// Prototypes
monster_t get_monsterdef(int n);
void simpleai(monster_t *m);
void advancedai(monster_t *m);

#endif
