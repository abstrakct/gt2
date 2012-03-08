/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * dealing with objects
 *
 * Copyright 2011 Rolf Klausen
 */

#ifndef _OBJECTS_H
#define _OBJECTS_H

#define MAX_EFFECTS  10
/*
typedef struct object_effect {
        struct object_effect *next;
        void   (*effect)(void *data);
} oe_t;
*/

struct object {
        //struct object *prev;
        struct object *head;
        struct object *next;
        int           id;                   // objdef-id
        int           oid;                  // unique id
        short         color;                // color!
        short         type;                 // see OT_defines below
        long          flags;                // 4 bytes = 32 bits/flags, see OF_defines below - CONSIDER CHANGE TO LONG LONG
        signed short  attackmod;            // +/- on attack; for armor: acmodifier
        signed short  damagemod;            // +/- on damage;
        char          basename[50];         // the basic name of the item
        char          unidname[100];        // unidentified name
        char          fullname[100];        // should be more than enough, adjust later
        char          displayname[128];
        char          c;
        char          slot;                 // inventory slot; not sure if needed?!
        char          minlevel;
        int           quantity;
        char          material;
        short         dice, sides;                 // sides is used for AC for armor!
        char          skill;                       // a particular skill needed to use this weapon?
        char          effects;
        short         effect[MAX_EFFECTS];
};

typedef struct object obj_t;

typedef struct {
        short num_used;
        int   gold;
        obj_t *object[52];
} inv_t;

// in inv_t:
// long long used;                   // use a bitfield/bitmap to note which slots are used? Worth it?

#define add_effect(a, b) if(a->effects < MAX_EFFECTS) { a->effect[(int)a->effects] = b; a->effects++; }

#define OT_GOLD       1
#define OT_WEAPON     2
#define OT_ARMOR      3
#define OT_BRACELET   4
#define OT_AMULET     5
#define OT_CARD       6
#define OT_WAND       7
#define OT_POTION     8

#define OBJECT_TYPES  8

extern char objchars[];
extern char *otypestrings[];

#define OF_MAGIC      0x00000001
#define OF_EATABLE    0x00000002
#define OF_ID_MOD     0x00000004      // set if modifier is known (e.g. if player knows that this is a +1 bracelet of whatever, not just a bracelet of whatever
#define OF_IDENTIFIED 0x00000008
#define OF_BAD        0x00000010
#define OF_GOOD       0x00000020
#define OF_INUSE      0x00000040
#define OF_UNIQUE     0x00000080

// Armor flags
#define OF_HEADGEAR   0x00000100
#define OF_BODYARMOR  0x00000200
#define OF_GLOVES     0x00000400
#define OF_FOOTWEAR   0x00000800
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

#define OF_OBVIOUS    0x01000000     // Effect is obvious -> identification!
#define OF_DONOTAP    0x02000000     // Do not autopickup this item!
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
#define MAT_COPPER     4
#define MAT_WOOD       5
#define MAT_IRON       6
#define MAT_MARBLE     7
#define MAT_GLASS      8
#define MAT_BONE       9
#define MAT_PLATINUM  10
#define MAT_STEEL     11
#define MAT_BLACKWOOD 12
#define MAT_BRASS     13
#define MAT_EBONY     14
#define MAT_BLOODWOOD 15

#define MATERIALS     15
extern int mats_bracelets[MATERIALS];
extern int mats_amulets[MATERIALS];

// defines so that we can easily use fields in obj_t for various stuff
#define ac sides

// and some nice macros
#define is_armor(a)         (a->type  == OT_ARMOR)
#define is_weapon(a)        (a->type  == OT_WEAPON)
#define is_bracelet(a)          (a->type  == OT_BRACELET)
#define is_amulet(a)        (a->type  == OT_AMULET)

#define do_identify(a)      (a->flags|= OF_IDENTIFIED)
#define is_identified(a)    (a->flags & OF_IDENTIFIED)
#define is_id_mod(a)        (a->flags & OF_ID_MOD)
#define is_magic(a)         (a->flags & OF_MAGIC)
#define is_eatable(a)       (a->flags & OF_EATABLE)
#define is_drinkable(a)     (a->flags & OF_DRINKABLE)
#define is_holyfuck(a)      (a->flags & OF_HOLYFUCK)
#define is_unique(a)        (a->flags & OF_UNIQUE)
#define is_headgear(a)      (a->flags & OF_HEADGEAR)
#define is_footwear(a)      (a->flags & OF_FOOTWEAR)
#define is_bodyarmor(a)     (a->flags & OF_BODYARMOR)
#define is_gloves(a)        (a->flags & OF_GLOVES)
#define is_shield(a)        (a->flags & OF_SHIELD)
#define obvious_effect(a)   (a->flags & OF_OBVIOUS)

#define unapply_effects apply_effects

// Prototypes
//

void   spawn_objects(int num, void *p);
bool   spawn_object_at(int y, int x, int n, void *level);
void   unspawn_object(obj_t *m);
obj_t *spawn_object(int n, void *level);
bool   place_object_at(obj_t *obj, int y, int x, void *p);
void   add_to_master_object_list(obj_t *o);
void   clear_master_object_list();

obj_t  get_objdef(int n);
int    get_objdef_by_name(char *wanted);
obj_t *get_object_by_oid(inv_t *i, int oid);

bool   is_pair(obj_t *o);
bool   is_worn(obj_t *o);      // worn by player, that is..

bool   move_to_inventory(obj_t *o, inv_t *i);
int    get_first_used_slot(inv_t *i);
int    get_next_used_slot_after(int n, inv_t *i);

void   pick_up(obj_t *o, void *a);
void   wieldwear(obj_t *o);
void   wield(obj_t *o);
void   unwieldwear(obj_t *o);
void   unwield(obj_t *o);
void   unwear(obj_t *o);
void   puton(int slot, obj_t *o);
void   drop(obj_t *o, void *actor);

inv_t *init_inventory();
void   spawn_golds(int num, int max, void *p);

void   init_objects();

// Autopickup!
void start_autopickup();
void stop_autopickup();
bool shall_autopickup(int type);


//void init_materials();
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
#define BRACELET(flags, base, mod) OBJ(OT_BRACELET, OF_MAGIC | flags, 0, base, "bracelet", mod, 0, 0, 0)
#define THING(flags, base, unid) OBJ(OT_THING, flags, 0, base, unid, 0, 0, 0, 0)

#define END_OBJECTS };
*/
