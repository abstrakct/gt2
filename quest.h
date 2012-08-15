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
        int  (*initiate)();                            // Basically, chatting with the NPC. Present quest, get a choice from the player.
        void (*countdown)(struct queststruct *quest);  // Countdown for timed quests
        void (*timeout_consequence)();                 // Perform whatever is the consequence if running out of time without completing the quest.
        bool (*fulfilled)();                           // Check if the quest's requirements/goals have been met.
        void (*finish)(struct queststruct *quest);     // Finish the quest (reward player or whatever).
} quest_t;

/* Global variables */
extern quest_t quest_garan_heidl;

/* Defines */
#define MAX_QUESTS 10

/* Prototypes */
void add_quest(void *npc);
void delete_quest(quest_t *quest);
void process_quests();
void show_player_quests();

#endif
