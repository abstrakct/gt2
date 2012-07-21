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

void add_quest(quest_t *quest)
{
        int i;

        i = 0;
        while(playerquests[i]) {
                i++;
                if(i >= MAX_QUESTS) {
                        gtprintf("You can't keep track of any more quests!");
                        break;
                }
        }

        playerquests[i] = quest;
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

bool quest_garan_heidl_initiate()
{
        gtkey key;

        gtmsgbox(" Chat ", "\"Hello stranger... My name is... Garan... Heidl... Don't know what's happened to the world... My legs are broken... Can't walk... Please, friend, fetch me something to drink! I haven't... much time left... So thirsty...\"\n\nWhat would you like to do?\na) Help the dying man.\nb) Leave him to die.");

        while(key.c != 'a' && key.c != 'b')
                key = gtgetch();

        if(key.c == 'a')
                return true;
        if(key.c == 'b')
                return false;

        return false;
}

void quest_countdown(quest_t *quest)
{
        if(quest->timer > 0) {
                quest->timer--;

                if(quest->timer <= 0) {
                        gtprintf("Oh no! Quest %s timed out!", quest->title);
                        delete_quest(quest);
                }
        }
}

quest_t quest_garan_heidl = {
        "Save a dying man.", "Garan Heidl wants you to fetch him something to drink before he dies of dehydration.", quest_garan_heidl_initiate, 2000, quest_countdown
};
// vim: fdm=syntax guifont=Terminus\ 8
