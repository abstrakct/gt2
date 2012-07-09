#ifndef _GT_SAVELOAD_H
#define _GT_SAVELOAD_H

#define SAVE_DIRECTORY ".saves"
#define GT_SAVEFILE_MAGIC  0xDEAD71FE

struct savefile_header {
        int magic;
        struct {
                int major;
                int minor;
                int revision;
        } version;
}; 

/* this one must match monster_t when it comes to variable types etc.! */
struct monsterdef_save_struct {
        short   id;
        char    name[50];
        uattr_t attr;
        char    c;
        int     level;
        int     hp;                 // == maxhp
        float   speed;
        long    flags;
        int     aitableindex;       // == the field mid
        short   viewradius;
        // what about AC?
};

/* This one must match obj_t when it comes to variable types etc.! */
struct objdef_save_struct {
        int          id;
        gtcolor_t    color;
        short        type;
        long         flags;
        signed short attackmod;
        signed short damagemod;
        char         basename[125];         // the basic (unidentified) name of the item
        char         fullname[125];        // should be more than enough, adjust later
        char         truename[125];
        char         displayname[125];
        char         c;
        char         minlevel;
        int          quantity;
        char         material;
        short        dice, sides;
        char         skill;
        char         effects;
        oe_t         effect[MAX_EFFECTS];
        short        rarity;
};

struct player_save_struct {
        short   x, y, oldx, oldy, px, py;
        short   viewradius;
        char    name[50];
        int     hp, maxhp;
        int     xp;
        short   ac;
        uattr_t attr;
        sattr_t attrmod;
        int     level;
        short   race, cla;
        int     weapon;              // save OID of weapon!
        int     w[WEAR_SLOTS];
        long    flags;
        double  speed;
        double  movement;
        char    wvfactor;
        short   worldview;
        float   skill[MAX_SKILLS];
        int     kills;
        int     temp[MAX_TEMP_EFFECTS];
};


void generate_savefilename(char *filename);
bool save_game(char *filename);
bool load_game(char *filename, int ingame);

#endif
