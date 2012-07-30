/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Quests.
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef _QUEST_H
#define _QUEST_H

/* Structs */
typedef struct queststruct {
        // Descriptions
        char *title;
        char *shortdescription;

        // Variables
        int  timer;
        bool quest_taken;
        bool quest_finished;

        // Functions
        int  (*initiate)();
        void (*countdown)(struct queststruct *quest);
        void (*timeout_consequence)();
        bool (*fulfilled)();
        void (*fulfill)(struct queststruct *quest);
} quest_t;

/* Global variables */
extern quest_t quest_garan_heidl;

/* Defines */
#define MAX_QUESTS 10

/* Prototypes */
void add_quest(void *npc);
void process_quests();

#endif
