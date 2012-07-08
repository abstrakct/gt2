/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * actors (player, monsters)
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef _ACTOR_H
#define _ACTOR_H

#define SKILL_SWORD    0
#define SKILL_KNIFE    1
#define SKILL_AXE      2
#define SKILL_STICK    3
#define SKILL_MACE     4

#define MAX_SKILLS 10

typedef struct { // sattr_t
        signed char str;
        signed char phy;
        signed char intl;
        signed char wis;
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

#define pstr player->attr.str
#define pphy player->attr.phy
#define pint player->attr.intl
#define pwis player->attr.wis
#define pdex player->attr.dex
#define pcha player->attr.cha

#define SLOT_HEAD          0
#define SLOT_BODY          1
#define SLOT_GLOVES        2
#define SLOT_FOOTWEAR      3
#define SLOT_CLOAK         4
#define SLOT_AMULET        5
#define SLOT_LEFTBRACELET  6
#define SLOT_RIGHTBRACELET 7
#define SLOT_SHIELD        8

#define pw_headgear      player->w[SLOT_HEAD]
#define pw_body          player->w[SLOT_BODY]
#define pw_gloves        player->w[SLOT_GLOVES]
#define pw_footwear      player->w[SLOT_FOOTWEAR]
#define pw_cloak         player->w[SLOT_CLOAK]
#define pw_amulet        player->w[SLOT_AMULET]
#define pw_leftbracelet  player->w[SLOT_LEFTBRACELET]
#define pw_rightbracelet player->w[SLOT_RIGHTBRACELET]
#define pw_shield        player->w[SLOT_SHIELD]

#define WEAR_SLOTS       10

#define pr_life      player->prot[0]
#define pr_fire      player->prot[1]
#define PROTECTIONS  2

#define is_invisible(a) (hasbit(a->flags, MF_INVISIBLE))
#define is_autoexploring (hasbit(player->flags, PF_AUTOEXPLORING))

#define TEMP_INVISIBLE    0
#define TEMP_STRENGTH     1
#define TEMP_WISDOM       2
#define TEMP_INTELLIGENCE 3
#define TEMP_CHARISMA     4
#define TEMP_DEXTERITY    5
#define TEMP_PHYSIQUE     6

#define TEMP_LAST         7
#define MAX_TEMP_EFFECTS 25

typedef struct actorstruct {                               // actor_t 
        short        id;                                   // id = monsterdef id
        unsigned int mid;                                  // mid = unique id for this monster; in monsterdefs mid = index into aitable! (that should work right?)
        short        x, y, oldx, oldy, px, py;
        short        viewradius;
        char         name[50];
        int          hp, maxhp;
        int          xp;
        short        ac;
        uattr_t      attr;
        sattr_t      attrmod;
        int          level;
        short        race, cla;
        inv_t        *inventory;
        obj_t        *weapon;                              // currently wielded weapon
        obj_t        *w[WEAR_SLOTS];                       // array rather than struct makes things easier!
        short        prot[PROTECTIONS];
        long         flags;
        int          c;                                    // character, for monsters.
        // TODO: Add variable for glyph color?!
        int          speed;
        float        skill[MAX_SKILLS];
        char         wvfactor;
        short        worldview;
        int          kills;
        //int          temp[MAX_TEMP_EFFECTS];                             // temporary effects
        oe_t         *effect[MAX_TEMP_EFFECTS];
        int          temp_effects;
#ifdef GT_USE_LIBTCOD
        TCOD_path_t  path;
#endif

        /* monster specific stuff */
        void               (*ai)(struct actorstruct *);    // artificial intelligence handler!!
        short              goalx, goaly;                   // for simple outdoor pathfinder ai
        struct actorstruct *prev;
        struct actorstruct *next;
        struct actorstruct *attacker;
        struct actorstruct *head;
} actor_t;

#define PF_AUTOEXPLORING (1 << 1)

char   get_first_free_letter();
char   slot_to_letter(int i);
int    letter_to_slot(char c);
obj_t *get_object_from_letter(char c, inv_t *i);
int    object_to_slot(obj_t *o, inv_t *inv);
void   assign_free_slot(obj_t *o);
void   unassign_object(obj_t *o);
void   assign_letter(char c, obj_t *o);

void autoexplore();

bool in_lineofsight(actor_t *src, int goaly, int goalx);
bool actor_in_lineofsight(actor_t *src, actor_t *dest);
bool next_to(actor_t *a, actor_t *b);
void attack(actor_t *attacker, actor_t *victim);
void increase_hp(actor_t *a, int amount);
void move_player_to_stairs_down(int d);
void move_player_to_stairs_up(int d);
int ability_modifier(int ab);

int get_strength(actor_t *a);
int get_wisdom(actor_t *a);
int get_dexterity(actor_t *a);
int get_physique(actor_t *a);
int get_intelligence(actor_t *a);
int get_charisma(actor_t *a);

#endif
