/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Saving and loading!
 *
 * Copyright 2011 Rolf Klausen
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "objects.h"
#include "actor.h"
#include "monsters.h"
#include "utils.h"
#include "datafiles.h"
#include "world.h"
#include "you.h"
#include "display.h"
#include "debug.h"
#include "saveload.h"
#include "gt.h"

/*
 * write one object
 */
void save_object(obj_t *o, FILE *f)
{
        fwrite("OBJECT", sizeof(char), 7, f);
}

/*
 * write one list of objects
 */
void save_inventory(obj_t *i, FILE *f)
{
        fwrite("INVENTORY", sizeof(char), 9, f);
}

/* 
 * write one monster
 */
void save_monster(monster_t *m, FILE *f)
{
        fwrite("MONSTER", sizeof(char),  7, f);     // is this necessary?
        fwrite(&m->id,    sizeof(short), 1, f);     // write the monsterdef ID, then any vars which might have changed
        fwrite(&m->mid,   sizeof(int),   1, f);
        fwrite(&m->y,     sizeof(short), 1, f);
        fwrite(&m->x,     sizeof(short), 1, f);
        fwrite(&m->hp,    sizeof(int),   1, f);
        fwrite(&m->goalx, sizeof(short), 1, f);
        fwrite(&m->goaly, sizeof(short), 1, f);

        // add saving inventory etc here later when implemented
        // add saving attacker, wear_t, etc.
}

/*
 * Write one cell_t  to file f, at current file position
 */
void save_cell(cell_t *c, FILE *f)
{
        bool hasmonster;

        fwrite(&c->type, sizeof(char), 1, f);
        fwrite(&c->flags, sizeof(int), 1, f);
        fwrite(&c->desty, sizeof(short), 1, f);
        fwrite(&c->destx, sizeof(short), 1, f);
        fwrite(&c->color, sizeof(short), 1, f);
        fwrite(&c->visible, sizeof(bool), 1, f);

        hasmonster = c->monster ? true : false;
        fwrite(&hasmonster, sizeof(bool), 1, f);
        if(hasmonster)
                save_monster(c->monster, f);
        hasmonster = c->inventory ? true : false;
        fwrite(&hasmonster, sizeof(bool), 1, f);
        if(c->inventory)
                save_inventory(c->inventory, f);

}

/*
 * Write one level_t to file f, at current file position
 */
void save_level(level_t *l, FILE *f)
{
        int y, x;

        fwrite("LEVEL", sizeof(char), 5, f);
        fwrite(&l->xsize, sizeof(short), 1, f);
        fwrite(&l->ysize, sizeof(short), 1, f);
        for(y = 0; y < l->ysize; y++)
                for(x = 0; x < l->xsize; x++)
                        save_cell(&l->c[y][x], f);
}

/*
 * Write one monsterdef
 */
void save_monsterdef(monster_t *m, FILE *f)
{
        struct monsterdef_save_struct s;

        s.id = m->id;
        strcpy(s.name, m->name);
        s.attr = m->attr;
        s.c = m->c;
        s.level = m->level;
        s.hp = m->hp;
        s.speed = m->speed;
        s.thac0 = m->thac0;
        s.flags = m->flags;
        s.aitableindex = m->mid;

        fwrite(&s, sizeof(struct monsterdef_save_struct), 1, f);
        gtprintf("saved monsterdef %s", s.name);
}

void save_objdef(obj_t *o, FILE *f)
{
        struct objdef_save_struct s;

        s.id = o->id;
        s.type = o->type;
        s.flags = o->flags;
        s.unique = o->unique;
        s.modifier = o->modifier;
        strcpy(s.basename, o->basename);
        strcpy(s.unidname, o->unidname);
        strcpy(s.fullname, o->fullname);
        s.c = o->c;
        s.minlevel = o->minlevel;
        s.quantity = o->quantity;
        s.material = o->material;
        s.ddice = o->ddice;
        s.dsides = o->dsides;
        s.skill = o->skill;

        fwrite(&s, sizeof(struct objdef_save_struct), 1, f);
        gtprintf("saved objdef %s", s.basename);
}

void save_player(actor_t *p, FILE *f)
{
        struct player_save_struct s;
        int i;

        s.x = p->x;
        s.y = p->y;
        s.oldx = p->oldx;
        s.oldy = p->oldy;
        s.px = p->px;
        s.py = p->py;
        s.viewradius = p->viewradius;
        strcpy(s.name, p->name);
        s.hp = p->hp;
        s.maxhp = p->maxhp;
        s.xp = p->xp;
        s.ac = p->ac;
        s.attr = p->attr;
        s.level = p->level;
        s.race = p->race;
        s.cla = p->cla;
        s.flags = p->flags;
        s.speed = p->speed;
        s.movement = p->movement;
        for(i = 0; i < MAX_SKILLS; i++)
                s.skill[i] = p->skill[i];

        /* TODO: ADD INVENTORY ETC */

        fwrite("PLAYER", sizeof(char), 6, f);
        fwrite(&s, sizeof(struct player_save_struct), 1, f);
}

void generate_savefilename(char *filename)
{
        sprintf(filename, "%s/%d.gtsave", SAVE_DIRECTORY, game->seed);
}

/*
 * Structure of the save file:
 *
 * - header struct
 * - gtconfig struct
 * - game struct
 * - the player (player_save_struct)
 * - the monsterdefs (monsterdef_save_struct)
 * - the objectdefs (objdef_save_struct)
 * - the levels
 */

bool save_game(char *filename)
{
//        char filename[255];
        char cmd[260];
        FILE *f;
        struct savefile_header header;
        //int i;
        monster_t *m;
        obj_t *o;

        gtprintf("Saving game to file %s", filename);
        f = fopen(filename, "w");

        header.magic = GT_SAVEFILE_MAGIC;
        header.version.major = GT_VERSION_MAJ;
        header.version.minor = GT_VERSION_MIN;
        header.version.revision = GT_VERSION_REV;

        fwrite(&header, sizeof(struct savefile_header), 1, f);
        fwrite(&gtconfig, sizeof(gt_config_t), 1, f);
        fwrite(game, sizeof(game_t), 1, f);

        /* then, let's save the player */
        save_player(player, f);

        m = monsterdefs->head;
        while(m) { 
                save_monsterdef(m, f);
                m = m->next;
        }

        o = objdefs->head;
        while(o) {
                save_objdef(o, f);
                o = o->next;
        }

        /* then, let's save world and levels */
        save_level(world->dng, f);

        fclose(f);

        if(gtconfig.compress_savefile) {
                sprintf(cmd, "xz %s", filename);
                system(cmd);
        }

        return true;
}

/*
 * And now... loading!
 */

void load_object(obj_t *o, FILE *f)
{
        char str[6];

        fread(str, sizeof(char), 6, f);
        gtprintf("load_object! str=%s", str);
}

void load_inventory(obj_t *i, FILE *f)
{
        char str[6];

        fread(str, sizeof(char), 9, f);
        gtprintf("load_inventory! str=%s", str);
}

void load_monster(monster_t *m, FILE *f)
{
        char str[7];

        fread(str, sizeof(char), 6, f);
        gtprintf("load_monster! STR=%s", str);
}

void load_cell(cell_t *c, FILE *f)
{
        bool flag;

        fread(&c->type,    sizeof(char), 1, f);
        fread(&c->flags,   sizeof(int), 1, f);
        fread(&c->desty,   sizeof(short), 1, f);
        fread(&c->destx,   sizeof(short), 1, f);
        fread(&c->color,   sizeof(short), 1, f);
        fread(&c->visible, sizeof(bool), 1, f);

        /* cell has monster? */
        fread(&flag, sizeof(bool), 1, f);
        if(flag)
                load_monster(c->monster, f);
        /* cell has inventory? */
        fread(&flag, sizeof(bool), 1, f);
        if(flag)
                load_inventory(c->inventory, f);
}

void load_level(level_t *l, FILE *f)
{
        int y, x;
        char str[5];

        fread(str, sizeof(char), 5, f);
        if(strcmp(str, "LEVEL")) {
                gtprintf("LEVEL header not were it's supposed to be!");
                return;
        }
        fread(&l->xsize, sizeof(short), 1, f);
        fread(&l->ysize, sizeof(short), 1, f);
        if(!l->c)
                init_level(l);

        for(y = 0; y < l->ysize; y++)
                for(x = 0; x < l->xsize; x++)
                        load_cell(&l->c[y][x], f);

        gtprintf("loaded level!");
}

/*
 * Load one monsterdef (into *m)
 */
void load_monsterdef(monster_t *m, FILE *f)
{
        struct monsterdef_save_struct s;

        fread(&s, sizeof(struct monsterdef_save_struct), 1, f);
        m->id = s.id;
        strcpy(m->name, s.name);
        m->attr = s.attr;
        m->c = s.c;
        m->level = s.level;
        m->hp = s.hp;
        m->speed = s.speed;
        m->thac0 = s.thac0;
        m->flags = s.flags;
        m->ai = aitable[s.aitableindex];
        gtprintf("loaded monsterdef %s", m->name);
}

void load_objdef(obj_t *o, FILE *f)
{
        struct objdef_save_struct s;

        fread(&s, sizeof(struct objdef_save_struct), 1, f);
        o->id = s.id;
        o->type = s.type;
        o->flags = s.flags;
        o->unique = s.unique;
        o->modifier = s.modifier;
        strcpy(o->basename, s.basename);
        strcpy(o->unidname, s.unidname);
        strcpy(o->fullname, s.fullname);
        o->c = s.c;
        o->minlevel = s.minlevel;
        o->quantity = s.quantity;
        o->material = s.material;
        o->ddice = s.ddice;
        o->dsides = s.dsides;
        o->skill = s.skill;
        gtprintf("loaded objdef %s", o->basename);
}

void load_player(actor_t *p, FILE *f)
{
        struct player_save_struct s;
        int i;
        char str[6];

        fread(str, sizeof(char), 6, f);
        if(strcmp(str, "PLAYER")) {
                fprintf(stderr, "DEBUG: %s:%d - str != PLAYER in load_player!\n", __FILE__, __LINE__);
                gtprintf("read %s instead of PLAYER?!", str);
                return;
                die("Player data not where it should be in savefile! Corrupt savefile?");
        }
        fread(&s, sizeof(struct player_save_struct), 1, f);
        p->x = s.x;
        p->y = s.y;
        p->oldx = s.oldx;
        p->oldy = s.oldy;
        p->px = s.px;
        p->py = s.py;
        p->viewradius = s.viewradius;
        strcpy(p->name, s.name);
        p->hp = s.hp;
        p->maxhp = s.maxhp;
        p->xp = s.xp;
        p->ac = s.ac;
        p->attr = s.attr;
        p->level = s.level;
        p->race = s.race;
        p->cla = s.cla;
        p->flags = s.flags;
        p->speed = s.speed;
        p->movement = s.movement;
        for(i = 0; i < MAX_SKILLS; i++)
                p->skill[i] = s.skill[i];

        /* TODO: ADD INVENTORY ETC */

}

bool load_game(char *filename, int ingame)
{
        FILE *f;
        struct savefile_header header;
        monster_t *m;
        obj_t *o;
        int i;

        gtprintf("Loading game from %s", filename);
        f = fopen(filename, "r");
        if(!f) {
                gtprintf("Couldn't open file %s", filename);
                return false;
        }

        fread(&header, sizeof(struct savefile_header), 1, f);
        if(header.magic != GT_SAVEFILE_MAGIC) {
                gtprintf("This doesn't look like a GT savefile to me!");
                return false;
        }
        if(header.version.major != GT_VERSION_MAJ || header.version.minor != GT_VERSION_MIN || header.version.revision != GT_VERSION_REV)
                gtprintf("Warning: Save file doesn't match current version number. This may or may not work...!");

        fread(&gtconfig, sizeof(gt_config_t), 1, f);
        fread(game, sizeof(game_t), 1, f);

        /* player */
        load_player(player, f);

        /* now, loading monsterdefs, linked lists, all that.. might be tricky! */
        for(i=0; i <= game->monsterdefs; i++) {
                if(!ingame)  // only alloc memory if we are loading at startup
                        m = gtmalloc(sizeof(monster_t));
                else
                        m = monsterdefs;
                load_monsterdef(m, f);
                
                m->head = monsterdefs->head;
                monsterdefs->next = m;
                m->next = NULL;
                m->prev = monsterdefs;
                monsterdefs = m;
        }

        /* objdefs */
        for(i=0; i <= game->objdefs; i++) {
                if(!ingame)  // only alloc memory if we are loading at startup
                        o = gtmalloc(sizeof(obj_t));
                else
                        o = objdefs;
                load_objdef(o, f);
                
                o->head = objdefs->head;
                objdefs->next = o;
                o->next = NULL;
                o->prev = objdefs;
                objdefs = o;
        }

        /* world */
        load_level(world->dng, f);

        fclose(f);
        return true;
}
