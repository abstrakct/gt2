#ifndef OBJECTS_H
#define OBJECTS_H

/*
 * dealing with objects
 */

struct object {
        struct object *prev;
        struct object *next;
        struct object *head;
        short type;             // see OT_defines below
        long flags;             // 4 bytes = 32 bits/flags, see OF_defines below
        char unique;            // is this an unique object in the world?
        signed short modifier;  // +/-, for armor: acmodifier
        char basename[50];      // the basic name of the item
        char unidname[100];     // unidentified name
        char fullname[100];     // should be more than enough, adjust later
        char c;
        char minlevel;
        short quantity;
        char material;
        int ddice, dsides;
        char skill;             // a particular skill needed to use this weapon?
};

typedef struct object obj_t;

#define OT_WEAPON     0
#define OT_ARMOR      1
#define OT_BODYARMOR  11
#define OT_RING       2
#define OT_CARD       3
#define OT_WAND       4
#define OT_THING      5
#define OT_GOLD       6

extern char objchars[];
extern char *otypestrings[];

#define OF_MAGIC      0x00000001
#define OF_EATABLE    0x00000002
#define OF_DRINKABLE  0x00000004
#define OF_IDENTIFIED 0x00000008
#define OF_BAD        0x00000010
#define OF_GOOD       0x00000020
#define OF_INUSE      0x00000040
#define OF_HEADARMOR  0x00000100
#define OF_BODYARMOR  0x00000200
#define OF_GLOVES     0x00000400
#define OF_FOOTARMOR  0x00000800
#define OF_SHIELD     0x00001000
#define OF_DONOTUSE   0x20000000
#define OF_HOLYFUCK   0x40000000

#define MAT_GOLD 1
#define MAT_SILVER 2
#define MAT_BRONZE 3
#define MAT_WOOD 4
#define MAT_IRON 5
#define MAT_COPPER 6
#define MAT_MARBLE 7
#define MAT_GLASS 8
#define MAT_BONE 9
#define MAT_PLATINUM 10
#define MAT_STEEL 11
#define MAT_BLACKWOOD 12
#define MAT_BRASS 13
#define MAT_EBONY 14
#define MATERIALS 14

// defines so that we can easily use fields in obj_t for various stuff
#define ac dsides

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


#define is_magic(a)     (a & OF_MAGIC)
#define is_eatable(a)   (a & OF_EATABLE)
#define is_drinkable(a) (a & OF_DRINKABLE)
#define identified(a)   (a & OF_IDENTIFIED)
#define is_holyfuck(a)  (a & OF_HOLYFUCK)
#define do_identify(a)   a |= OF_IDENTIFIED
#define is_headwear(a)  (a & OF_HEADARMOR)
#define is_footwear(a)  (a & OF_FOOTARMOR)
#define is_bodywear(a)  (a & OF_BODYARMOR)
#define is_gloves(a)    (a & OF_GLOVES)
#define is_shield(a)    (a & OF_SHIELD)

//void init_objects();
//void init_materials();
//char *get_def_name(obj_t object);
//char *a_an(char *s);
//void uppercase(char *s);
//void moveobject(obj_t *src, obj_t *dest);
//int wieldable(obj_t *obj);
//int wearable(obj_t *obj);

#endif
