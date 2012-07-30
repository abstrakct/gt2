/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Quests.
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
#include "quest.h"
#include "npc.h"
#include "world.h"
#include "datafiles.h"
#include "io.h"
#include "debug.h"
#include "gt.h"
#include "utils.h"

void add_quest(void *npc)
{
        int i;
        npc_t *n;

        n = (npc_t *)npc;

        i = 0;
        while(playerquests[i]) {
                i++;
                if(i >= MAX_QUESTS) {
                        gtprintf("You can't keep track of any more quests!");
                        break;
                }
        }

        playerquests[i] = n->quest;
        n->quest->quest_taken = true;
        n->quest_taken = true;
}

void delete_quest(quest_t *quest)
{
        int i;

        for(i = 0; i < MAX_QUESTS; i++) {
                if(playerquests[i] == quest)
                        playerquests[i] = NULL;
        }
}

void process_quests()
{
        int i;

        for(i = 0; i < MAX_QUESTS; i++) {
                if(playerquests[i]) {
                        if(playerquests[i]->countdown)
                                playerquests[i]->countdown(playerquests[i]);
                }
        }
}

int quest_garan_heidl_initiate()
{
        gtkey key;

        if(hasbit(predef_npcs[NPC_GARAN_HEIDL].flags, MF_ISDEAD)) {
                gtprintf("The dried up corpse of %s has nothing more to say to you.", predef_npcs[NPC_GARAN_HEIDL].name);
                return -1;
        }

        if(predef_npcs[NPC_GARAN_HEIDL].quest->quest_taken) {
                gtprintf("The dehydrated man seems to be in too much pain to talk.");
                return -1;
        }

        gtmsgbox(" Chat ", "\"Hello stranger... My name is... Garan... Heidl... I don't know what's happened to the world... I fell down a shaft, broke my legs... Can't walk... Please, friend, fetch me something to drink! I haven't much... time... left... Oh, the pain! So thirsty...\"\n\nWhat would you like to do?\na) Help the dying man.\nb) Leave him to die.\n");

        while(key.c != 'a' && key.c != 'b')
                key = gtgetch();

        if(key.c == 'a')
                return 1;
        if(key.c == 'b')
                return 0;

        return 0;
}

void quest_garan_heidl_timeout()
{
        setbit(predef_npcs[NPC_GARAN_HEIDL].flags, MF_ISDEAD);
        // drop inventory!?!?
}

bool quest_garan_heidl_fulfilled()
{
        if(has_in_inventory(player, "bottle of water"))
                return true;
        if(has_in_inventory(player, "bottle of ale"))
                return true;
        if(has_in_inventory(player, "bottle of brown ale"))
                return true;
        if(has_in_inventory(player, "bottle of wine"))
                return true;
        if(has_in_inventory(player, "bottle of mead"))
                return true;

        return false;
}

void quest_garan_heidl_fulfill(quest_t *quest)
{
        obj_t *o, *o2, *o3, *o4, *o5;
        char m[1000];

        if(hasbit(predef_npcs[NPC_GARAN_HEIDL].flags, MF_ISDEAD))
                gtprintf("The dried up corpse of %s has nothing more to say to you.", predef_npcs[NPC_GARAN_HEIDL].name);

        if(!quest->quest_finished) {
                if(has_in_inventory(player, "bottle of mead")) {
                        sprintf(m, "Garan Heidl slowly drinks the mead. \"Ah, thank you friend, for this orange blossom mead! A nice way to end my life... Please take these items as a token of my gratitude.\" Garan Heidl's head drops to one side. He seems to have stopped breathing.");
                        gtmsgbox(" Chat ", m);

                        o  = spawn_object_with_rarity(VERYCOMMON, world->curlevel);
                        o2 = spawn_object_with_rarity(VERYCOMMON, world->curlevel);
                        o3 = spawn_object_with_rarity(COMMON, world->curlevel);
                        o4 = spawn_object_with_rarity(COMMON, world->curlevel);
                        o5 = spawn_object_with_rarity(COMMON, world->curlevel);
                        move_to_cell_inventory(o,  world->curlevel, predef_npcs[NPC_GARAN_HEIDL].y, predef_npcs[NPC_GARAN_HEIDL].x);
                        move_to_cell_inventory(o2, world->curlevel, predef_npcs[NPC_GARAN_HEIDL].y, predef_npcs[NPC_GARAN_HEIDL].x);
                        move_to_cell_inventory(o3, world->curlevel, predef_npcs[NPC_GARAN_HEIDL].y, predef_npcs[NPC_GARAN_HEIDL].x);
                        move_to_cell_inventory(o4, world->curlevel, predef_npcs[NPC_GARAN_HEIDL].y, predef_npcs[NPC_GARAN_HEIDL].x);
                        move_to_cell_inventory(o5, world->curlevel, predef_npcs[NPC_GARAN_HEIDL].y, predef_npcs[NPC_GARAN_HEIDL].x);

                        setbit(predef_npcs[NPC_GARAN_HEIDL].flags, MF_ISDEAD);
                } else if(has_in_inventory(player, "bottle of wine")) {
                        sprintf(m, "Garan Heidl slowly drinks the bottle wine. \"Ah, thank you friend, for this spiced wine! A fitting way to end my life... Please take these items as a token of my gratitude.\" Garan Heidl's head drops to one side. He seems to have stopped breathing.");
                        gtmsgbox(" Chat ", m);

                        o  = spawn_object_with_rarity(VERYCOMMON, world->curlevel);
                        o2 = spawn_object_with_rarity(VERYCOMMON, world->curlevel);
                        o3 = spawn_object_with_rarity(COMMON, world->curlevel);
                        o4 = spawn_object_with_rarity(COMMON, world->curlevel);
                        move_to_cell_inventory(o,  world->curlevel, predef_npcs[NPC_GARAN_HEIDL].y, predef_npcs[NPC_GARAN_HEIDL].x);
                        move_to_cell_inventory(o2, world->curlevel, predef_npcs[NPC_GARAN_HEIDL].y, predef_npcs[NPC_GARAN_HEIDL].x);
                        move_to_cell_inventory(o3, world->curlevel, predef_npcs[NPC_GARAN_HEIDL].y, predef_npcs[NPC_GARAN_HEIDL].x);
                        move_to_cell_inventory(o4, world->curlevel, predef_npcs[NPC_GARAN_HEIDL].y, predef_npcs[NPC_GARAN_HEIDL].x);

                        setbit(predef_npcs[NPC_GARAN_HEIDL].flags, MF_ISDEAD);
                } else if(has_in_inventory(player, "bottle of brown ale")) {    // TODO: PLACE NAMES FOR DRINKS!!!
                        sprintf(m, "Garan Heidl slowly drinks the brown ale. \"Ah, thank you friend, for this Armyllen Brown Ale! A quite nice way to end my life... Please take these items as a token of my gratitude.\" Garan Heidl's head drops to one side. He seems to have stopped breathing.");
                        gtmsgbox(" Chat ", m);

                        o  = spawn_object_with_rarity(VERYCOMMON, world->curlevel);
                        o2 = spawn_object_with_rarity(VERYCOMMON, world->curlevel);
                        o3 = spawn_object_with_rarity(COMMON, world->curlevel);
                        move_to_cell_inventory(o,  world->curlevel, predef_npcs[NPC_GARAN_HEIDL].y, predef_npcs[NPC_GARAN_HEIDL].x);
                        move_to_cell_inventory(o2, world->curlevel, predef_npcs[NPC_GARAN_HEIDL].y, predef_npcs[NPC_GARAN_HEIDL].x);
                        move_to_cell_inventory(o3, world->curlevel, predef_npcs[NPC_GARAN_HEIDL].y, predef_npcs[NPC_GARAN_HEIDL].x);

                        setbit(predef_npcs[NPC_GARAN_HEIDL].flags, MF_ISDEAD);
                } else if(has_in_inventory(player, "bottle of amber ale")) {
                        sprintf(m, "Garan Heidl slowly drinks the ale. \"Ah, thank you friend, for this ale! It reminds me of when I was younger, the long summer days of working on my uncle's farm... Please take these items as a token of my gratitude.\" Garan Heidl's head drops to one side. He seems to have stopped breathing.");
                        gtmsgbox(" Chat ", m);

                        o  = spawn_object_with_rarity(VERYCOMMON, world->curlevel);
                        o2 = spawn_object_with_rarity(VERYCOMMON, world->curlevel);
                        move_to_cell_inventory(o,  world->curlevel, predef_npcs[NPC_GARAN_HEIDL].y, predef_npcs[NPC_GARAN_HEIDL].x);
                        move_to_cell_inventory(o2, world->curlevel, predef_npcs[NPC_GARAN_HEIDL].y, predef_npcs[NPC_GARAN_HEIDL].x);

                        setbit(predef_npcs[NPC_GARAN_HEIDL].flags, MF_ISDEAD);
                } else if(has_in_inventory(player, "bottle of water")) {
                        o = spawn_object_with_rarity(VERYCOMMON, world->curlevel);
                        move_to_inventory(o, player->inventory);
                        sprintf(m, "Garan Heidl gulps down the water. \"Oh, thank you my friend, for this bottle of water! Here, take this %s! I sure won't need it anymore.\"", o->displayname);
                        gtmsgbox(" Chat ", m);
                }

                quest->quest_finished = true;
        }
}

void quest_countdown(quest_t *quest)
{
        if(quest->timer > 0) {
                quest->timer--;

                if(quest->timer <= 0) {
                        //gtprintf("Oh no! Quest %s timed out!", quest->title);

                        quest->timeout_consequence();
                        delete_quest(quest);
                }
        }
}

quest_t quest_garan_heidl = {
        "Save a dying man.", "Garan Heidl wants you to fetch him something to drink before he dies of dehydration.",\
                2500, false, false,\
                quest_garan_heidl_initiate, quest_countdown, quest_garan_heidl_timeout, quest_garan_heidl_fulfilled, quest_garan_heidl_fulfill
};
// vim: fdm=syntax guifont=Terminus\ 8
