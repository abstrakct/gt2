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

        gtmsgbox(" Chat ", "\"Hello stranger... My name is... Garan... Heidl... Don't know what's happened to the world... My legs are broken... Can't walk... Please, friend, fetch me something to drink! I haven't... much time left... So thirsty...\"\n\nWhat would you like to do?\na) Help the dying man.\nb) Leave him to die.\n");

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
        "Save a dying man.", "Garan Heidl wants you to fetch him something to drink before he dies of dehydration.", quest_garan_heidl_initiate, 2000, quest_countdown, quest_garan_heidl_timeout
};
// vim: fdm=syntax guifont=Terminus\ 8
