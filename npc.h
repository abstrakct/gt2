/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * NPCs.
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef _NPC_H
#define _NPC_H

typedef struct npc_struct {
        char  name[100];
        bool  spawned;
        bool  has_quest;
        bool  unique;
        void  (*chat)(struct npc_struct *talker);
        short level;
} npc_t;

extern npc_t predef_npcs[];

// npc functions
int get_number_of_npcs_nearby(actor_t *actor);
npc_t* get_nearest_npc(actor_t *a);
void spawn_predef_npcs();

// chat functions
void chat_hello(struct npc_struct *talker);

#endif
