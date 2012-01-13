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
        fwrite("OBJECT", sizeof(char), 6, f);
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
}

void save_player(actor_t *p, FILE *f)
{
        struct player_save_struct s;
        int i;

        s.x = p->x;
        s.y = p->x;
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
 * - the monsterdefs
 * - the objectdefs
 * - the player
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
        fwrite(&game, sizeof(game_t), 1, f);

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

        /* then, let's save the player */
        save_player(player, f);

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
}

/*
 * And now... loading!
 */

bool load_game(char *filename)
{
        FILE *f;
        struct savefile_header header;
        monster_t *m;
        int i;

        gtprintf("Loading game from %s", filename);
        f = fopen(filename, "r");

        fread(&header, sizeof(struct savefile_header), 1, f);
        if(header.magic != GT_SAVEFILE_MAGIC) {
                gtprintf("This doesn't look like a GT savefile to me!");
                return false;
        }
        if(header.version.major != GT_VERSION_MAJ || header.version.minor != GT_VERSION_MIN || header.version.revision != GT_VERSION_REV)
                gtprintf("Warning: Save file doesn't match current version number. This may or may not work...!");

        fread(&gtconfig, sizeof(gt_config_t), 1, f);
        fread(&game, sizeof(game_t), 1, f);

        /* now, loading monsterdefs, linked lists, all that.. might be tricky! */
        for(i=0; i < game->monsterdefs; i++) {
                m = gtmalloc(sizeof(monster_t));
                load_monsterdef(m, f);
                
                m->head = monsterdefs->head;
                monsterdefs->next = m;
                m->next = NULL;
                m->prev = monsterdefs;
                monsterdefs = m;
        }

        fclose(f);
        return true;
}
