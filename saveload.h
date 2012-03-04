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
       int          color;
       short        type;
       long         flags;
       signed short attackmod;
       signed short damagemod;
       char         basename[50];
       char         unidname[100];
       char         fullname[100];   // we might want to not include these here...??
       char         c;
       char         minlevel;
       int          quantity;
       char         material;
       short        dice, sides;
       char         skill;
};

struct player_save_struct {
        short   x, y, oldx, oldy, px, py;
        short   viewradius;
        char    name[50];
        int     hp, maxhp;
        int     xp;
        short   ac;
        uattr_t attr;
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
};


void generate_savefilename(char *filename);
bool save_game(char *filename);
bool load_game(char *filename, int ingame);

#endif
