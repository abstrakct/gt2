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
        char *title;
        char *shortdescription;
        bool (*initiate)();
        int  timer;
        void (*countdown)(struct queststruct *quest);
} quest_t;

/* Global variables */
extern quest_t quest_garan_heidl;

/* Defines */
#define MAX_QUESTS 10

/* Prototypes */
void add_quest(quest_t *quest);
void process_quests();

#endif
