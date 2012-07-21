/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * NPCs.
 *
 * Copyright 2011 Rolf Klausen
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "objects.h"
#include "o_effects.h"
#include "actor.h"
#include "monsters.h"
#include "npc.h"
#include "world.h"
#include "datafiles.h"
#include "io.h"
#include "debug.h"
#include "gt.h"
#include "utils.h"

npc_t predef_npcs[] = {
        // Name          spawned, has_quest, unique, chat func, level,  
        { "Garan Heidl", false,   false,     true,   chat_hello,  1,  }
};

void spawn_predef_npcs()
{
        int i, max, x, y;
        npc_t *npc;

        max = sizeof(predef_npcs) / sizeof(npc_t);

        for(i = 0; i < max; i++) {
                npc = &predef_npcs[i];
                if(npc->spawned)
                        break;

                printf("Spawning predef npc %d\n", i);

                x = y = 0;
                while(!passable(&world->dng[npc->level], y, x)) {
                        x = ri(1, world->dng[npc->level].xsize - 1);
                        y = ri(1, world->dng[npc->level].ysize - 1);
                }

                world->dng[npc->level].c[y][x].npc = npc;
                npc->spawned = true;
        }
}

npc_t* get_nearest_npc(actor_t *actor)
{
        if(world->curlevel->c[actor->y][actor->x - 1].npc)
                return world->curlevel->c[actor->y][actor->x - 1].npc;
        if(world->curlevel->c[actor->y][actor->x + 1].npc)
                return world->curlevel->c[actor->y][actor->x + 1].npc;
        if(world->curlevel->c[actor->y - 1][actor->x].npc)
                return world->curlevel->c[actor->y - 1][actor->x].npc;
        if(world->curlevel->c[actor->y + 1][actor->x].npc)
                return world->curlevel->c[actor->y + 1][actor->x].npc;
        if(world->curlevel->c[actor->y - 1][actor->x - 1].npc)
                return world->curlevel->c[actor->y - 1][actor->x - 1].npc;
        if(world->curlevel->c[actor->y - 1][actor->x + 1].npc)
                return world->curlevel->c[actor->y - 1][actor->x + 1].npc;
        if(world->curlevel->c[actor->y + 1][actor->x - 1].npc)
                return world->curlevel->c[actor->y + 1][actor->x - 1].npc;
        if(world->curlevel->c[actor->y + 1][actor->x + 1].npc)
                return world->curlevel->c[actor->y + 1][actor->x + 1].npc;

        return 0;
}

int get_number_of_npcs_nearby(actor_t *actor)
{
        int i;

        i = 0;
        if(world->curlevel->c[actor->y][actor->x - 1].npc)
                i++;
        if(world->curlevel->c[actor->y][actor->x + 1].npc)
                i++;
        if(world->curlevel->c[actor->y - 1][actor->x].npc)
                i++;
        if(world->curlevel->c[actor->y + 1][actor->x].npc)
                i++;
        if(world->curlevel->c[actor->y - 1][actor->x - 1].npc)
                i++;
        if(world->curlevel->c[actor->y - 1][actor->x + 1].npc)
                i++;
        if(world->curlevel->c[actor->y + 1][actor->x - 1].npc)
                i++;
        if(world->curlevel->c[actor->y + 1][actor->x + 1].npc)
                i++;

        return i;
}

void chat_hello(struct npc_struct *talker)
{
        char m[100];
        sprintf(m, "%s says: \"Hello there %s!\"", talker->name, player->name);

        gtmsgbox(" Chat ", m);
}
