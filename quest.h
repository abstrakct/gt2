/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Quests.
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef _QUEST_H
#define _QUEST_H

typedef struct queststruct {
        char *title;
        char *shortdescription;
        bool (*initiate)();
} quest_t;

extern quest_t quest_garan_heidl;

#endif
