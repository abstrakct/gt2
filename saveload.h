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
        int     thac0;
        long    flags;
        int     aitableindex;       // == the field mid
        // what about AC?
};

/* This one must match obj_t when it comes to variable types etc.! */
struct objdef_save_struct {
       int          id;
       short        type;
       long         flags;
       char         unique;
       signed short modifier;
       char         basename[50];
       char         unidname[100];
       char         fullname[100];   // we might want to not include these here...??
       char         c;
       char         minlevel;
       short        quantity;
       char         material;
       int          ddice, dsides;
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
        //wear_t  w;
        long    flags;
        double  speed;
        double  movement;
        int     thac0;
        float   skill[MAX_SKILLS];
};


void generate_savefilename(char *filename);
bool save_game(char *filename);
bool load_game(char *filename, int ingame);

#endif
