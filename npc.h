/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * NPCs.
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef _NPC_H
#define _NPC_H

typedef struct {
        char name[100];
        bool has_quest;
        void (*chat)();
} npc_t;

extern npc_t predef_npcs[];

// npc functions
int get_number_of_npcs_nearby(actor_t *actor);
npc_t* get_nearest_npc(actor_t *a);

// chat functions
void chat_garan_heidl();

#endif
