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
        fwrite(&m->y,     sizeof(short), 1, f);
        fwrite(&m->x,     sizeof(short), 1, f);
        fwrite(&m->hp,    sizeof(int),   1, f);
        fwrite(&m->goalx, sizeof(short), 1, f);
        fwrite(&m->goaly, sizeof(short), 1, f);

        // add saving inventory etc here later when implemented
}

/*
 * Write one cell_t  to file f, at current file position
 */
void save_cell(FILE *f, cell_t *c)
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
        if(c->inventory)
                save_inventory(c->inventory, f);
}

/*
 * Write one level_t to file f, at current file position
 */
void save_level(FILE *f, level_t *l)
{
        int y, x;

fprintf(stderr, "DEBUG: %s:%d - writing level of size %d,%d\n", __FILE__, __LINE__, l->xsize, l->ysize);
        fwrite(&l->xsize, sizeof(short), 1, f);
        fwrite(&l->ysize, sizeof(short), 1, f);
        for(y = 0; y < l->ysize; y++)
                for(x = 0; x < l->xsize; x++)
                        save_cell(f, &l->c[y][x]);
}

/*
 * Structure of the save file:
 *
 * - header struct
 * - gtconfig struct
 * - game struct
 * - the monsterdefs (it also saves all pointers! this must (if possible) be fixed when loading!!
 * - the objectdefs  (same)
 *
 */

bool save_game()
{
        char filename[255];
        char cmd[260];
        FILE *f;
        struct savefile_header header;
        //int i;
        monster_t *m;
        obj_t *o;

        sprintf(filename, "%s/%d.gtsave", SAVE_DIRECTORY, game->seed);
        gtprintf("Saving game to file %s", filename);
        f = fopen(filename, "w");

        header.magic = 0xDEAD71FE;
        header.version.major = GT_VERSION_MAJ;
        header.version.minor = GT_VERSION_MIN;
        header.version.revision = GT_VERSION_REV;

        fwrite(&header, sizeof(struct savefile_header), 1, f);
        fwrite(&gtconfig, sizeof(gt_config_t), 1, f);
        fwrite(&game, sizeof(game_t), 1, f);

        m = monsterdefs->head;
        while(m) { // maybe a function specifically for writing a monsterdef? and objdef
                fwrite(m, sizeof(monster_t), 1, f);
                m = m->next;
        }

        o = objdefs->head;
        while(o) {
                fwrite(o, sizeof(obj_t), 1, f);
                o = o->next;
        }

        /* then, lets save world and levels */
        save_level(f, world->dng);

        fclose(f);

        if(gtconfig.compress_savefile) {
                sprintf(cmd, "xz %s", filename);
                system(cmd);
        }

        return true;
}

bool load_game()
{

        return true;
}
