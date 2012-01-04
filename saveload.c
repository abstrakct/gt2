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
        int i;
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
        for(i=0; i < game->monsterdefs; i++) {
                while(m) {
                        fwrite(m, sizeof(monster_t), 1, f);
                        m = m->next;
                }
        }

        o = objdefs->head;
        for(i=0; i < game->objdefs; i++) {
                while(o) {
                        fwrite(o, sizeof(obj_t), 1, f);
                        o = o->next;
                }
        }

        /* then, lets save world and levels */
        

        fclose(f);
        sprintf(cmd, "xz %s", filename);
        system(cmd);

        return true;
}

bool load_game()
{

        return true;
}
