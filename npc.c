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
        { "Garan Heidl", false, chat_garan_heidl }
};

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

void chat_garan_heidl()
{
        gtprintf("Garan Heidl says: \"Hello there %s!\"", player->name);
}
