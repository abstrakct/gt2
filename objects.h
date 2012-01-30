/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * dealing with objects
 *
 * Copyright 2011 Rolf Klausen
 */

#ifndef _OBJECTS_H
#define _OBJECTS_H

struct object {
        struct object *prev;
        struct object *next;
        struct object *head;
        int  id;                // objdef-id
        int oid;                // unique id
        short type;             // see OT_defines below
        long flags;             // 4 bytes = 32 bits/flags, see OF_defines below - CONSIDER CHANGE TO LONG LONG
        signed short attackmod; // +/- on attack; for armor: acmodifier
        signed short damagemod; // +/- on damage;
        char basename[50];      // the basic name of the item
        char unidname[100];     // unidentified name
        char fullname[100];     // should be more than enough, adjust later
        char c;
        char minlevel;
        short quantity;
        char material;
        short dice, sides;      // sides is used for AC for armor!
        char skill;             // a particular skill needed to use this weapon?
};

typedef struct object obj_t;

#define OT_WEAPON     1
#define OT_ARMOR      2
#define OT_RING       3
#define OT_CARD       4
#define OT_WAND       5
#define OT_THING      6
#define OT_GOLD       7
#define OT_AMULET     8
#define OT_POTION     9


extern char objchars[];
extern char *otypestrings[];

#define OF_MAGIC      0x00000001
#define OF_EATABLE    0x00000002
#define OF_DRINKABLE  0x00000004
#define OF_IDENTIFIED 0x00000008
#define OF_BAD        0x00000010
#define OF_GOOD       0x00000020
#define OF_INUSE      0x00000040
#define OF_UNIQUE     0x00000080

// Armor flags
#define OF_HEADARMOR  0x00000100
#define OF_BODYARMOR  0x00000200
#define OF_GLOVES     0x00000400
#define OF_FOOTARMOR  0x00000800
#define OF_SHIELD     0x00001000

// Weapon flags
#define OF_SWORD      0x00010000
#define OF_AXE        0x00020000
#define OF_KNIFE      0x00040000
#define OF_STICK      0x00080000   /// ???
#define OF_MACE       0x00100000
#define OF_HAMMER     0x00200000
//#define OF_UNUSED1    0x00400000   // do we need more flags?!?! long long or?
#define OF_TWOHANDED  0x00800000

//#define OF_UNUSED2    0x01000000
//#define OF_UNUSED3    0x02000000
//#define OF_UNUSED4    0x04000000
//#define OF_UNUSED5    0x08000000
//#define OF_UNUSED6    0x10000000
//#define OF_UNUSED7    0x20000000

// Special flags!
#define OF_SPAWNED    0x40000000        // use to set if a unique object has been spawned or not!
#define OF_HOLYFUCK   0x80000000

#define MAT_GOLD       1
#define MAT_SILVER     2
#define MAT_BRONZE     3
#define MAT_WOOD       4
#define MAT_IRON       5
#define MAT_COPPER     6
#define MAT_MARBLE     7
#define MAT_GLASS      8
#define MAT_BONE       9
#define MAT_PLATINUM  10
#define MAT_STEEL     11
#define MAT_BLACKWOOD 12
#define MAT_BRASS     13
#define MAT_EBONY     14
#define MATERIALS     14

// defines so that we can easily use fields in obj_t for various stuff
#define ac sides

// and some nice macros
#define identified(a)   (a & OF_IDENTIFIED)
#define do_identify(a)   a|= OF_IDENTIFIED
#define is_armor(a)     (a == OT_ARMOR)
#define is_weapon(a)    (a == OT_WEAPON)
#define is_magic(a)     (a & OF_MAGIC)
#define is_eatable(a)   (a & OF_EATABLE)
#define is_drinkable(a) (a & OF_DRINKABLE)
#define is_holyfuck(a)  (a & OF_HOLYFUCK)
#define is_unique(a)    (a & OF_UNIQUE)
#define is_headwear(a)  (a & OF_HEADARMOR)
#define is_footwear(a)  (a & OF_FOOTARMOR)
#define is_bodywear(a)  (a & OF_BODYARMOR)
#define is_gloves(a)    (a & OF_GLOVES)
#define is_shield(a)    (a & OF_SHIELD)

// Prototypes
//

void spawn_objects(int num, void *p);
bool spawn_object_at(int y, int x, int n, obj_t *head, void *level);
void unspawn_object(obj_t *m);
void spawn_object(int n, obj_t *head);
bool place_object_at(int y, int x, obj_t *obj, void *l);

obj_t get_objdef(int n);
int get_objdef_by_name(char *wanted);
bool is_pair(obj_t *o);

bool move_to_inventory(obj_t *o, obj_t *i);
void pick_up(obj_t *o, void *a);

obj_t *init_inventory();
void spawn_golds(int num, int max, void *p);

//void init_objects();
//void init_materials();
//char *get_def_name(obj_t object);
//char *a_an(char *s);
//void uppercase(char *s);
//void moveobject(obj_t *src, obj_t *dest);
//int wieldable(obj_t *obj);
//int wearable(obj_t *obj);

#endif
/*
struct obj_list {
        obj_t *object;
        struct obj_list *next;
};

typedef struct obj_list obj_l;

#define START_OBJECTS obj_t objects[] = {

#define OBJ(type, flags, unique, base, unid, mod, ddice, dsides, skill) { 0, 0, type, flags, unique, mod, base, unid, "\0", 0, 1, 1, 0, ddice, dsides, skill},

#define WEAPON(flags, base, unid, ddice, dsides, skill) OBJ(OT_WEAPON, flags, 0, base, unid, 0, ddice, dsides, skill)
#define WEAPON_UNIQUE(flags, base, unid, mod, ddice, dsides, skill) OBJ(OT_WEAPON, flags, 1, base, unid, mod, ddice, dsides, skill)
#define MAGIC_WEAPON(flags, base, unid, mod, ddice, dsides, skill) OBJ(OT_WEAPON, OF_MAGIC | flags, 0, base, unid, mod, ddice, dsides, skill)
#define MAGIC_WEAPON_UNIQUE(flags, base, unid, mod, ddice, dsides, skill) OBJ(OT_WEAPON, OF_MAGIC | flags, 1, base, unid, mod, ddice, dsides, skill)

#define ARMOR(flags, base, unid, ac)  OBJ(OT_ARMOR, flags, 0, base, unid, 0, 0, ac, 0)
#define SHIELD(flags, base, unid, ac) OBJ(OT_ARMOR, flags | OF_SHIELD, 0, base, unid, 0, 0, ac, 0)
#define HEADARMOR(flags, base, unid, ac) OBJ(OT_ARMOR, flags | OF_HEADARMOR, 0, base, unid, 0, 0, ac, 0)
#define GLOVEARMOR(flags, base, unid, ac) OBJ(OT_ARMOR, flags | OF_GLOVES, 0, base, unid, 0, 0, ac, 0)
#define FOOTARMOR(flags, base, unid, ac) OBJ(OT_ARMOR, flags | OF_FOOTARMOR, 0, base, unid, 0, 0, ac, 0)
#define MAGIC_ARMOR(flags, base, unid, ac, mod) OBJ(OT_ARMOR, OF_MAGIC | flags, 0, base, unid, mod, 0, ac, 0)
#define CARD(flags, base, unid) OBJ(OT_CARD, OF_MAGIC | flags, 1, base, unid, 0, 0, 0, 0)
#define WAND(flags, base) OBJ(OT_WAND, OF_MAGIC | flags, 0, base, "wand", 0, 0, 0, 0)
#define RING(flags, base, mod) OBJ(OT_RING, OF_MAGIC | flags, 0, base, "ring", mod, 0, 0, 0)
#define THING(flags, base, unid) OBJ(OT_THING, flags, 0, base, unid, 0, 0, 0, 0)

#define END_OBJECTS };
*/
