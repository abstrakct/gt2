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

bool quest_garan_heidl_initiate()
{
        gtmsgbox(" Chat ", "\"Hello stranger... My name is... Garan... Heidl... Don't know what's happened to the world... My legs are broken... Can't walk... Please, friend, fetch me something to drink! I haven't... much time left... So thirsty...\"");

        return true;
}

quest_t quest_garan_heidl = {
        "Save a dying man.", "Garan Heidl wants you to fetch him something to drink before he dies of dehydration.", quest_garan_heidl_initiate
};
// vim: fdm=syntax guifont=Terminus\ 8
