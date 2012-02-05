/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Object effects
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
#include "utils.h"
#include "datafiles.h"
#include "world.h"
#include "display.h"
#include "debug.h"
#include "gt.h"

                
void oe_strength(void *data)
{
        obj_t *o;
        int x;

        o = (obj_t *) data;
        x = player->attr.str;

        if(!is_worn(o)) {
                player->attr.str += o->attackmod;
        } else {
                player->attr.str -= o->attackmod;
        }

        if(x > player->attr.str)
                you("feel weaker!");
        else
                you("feel stronger!");
}

void oe_wisdom(void *data)
{
        obj_t *o;
        int x;

        o = (obj_t *) data;
        x = player->attr.wis;

        if(!is_worn(o)) {
                player->attr.wis += o->attackmod;
        } else {
                player->attr.wis -= o->attackmod;
        }

        if(x > player->attr.wis)
                you("feel more ignorant!");
        else
                you("feel wiser!");
}

void oe_physique(void *data)
{
        obj_t *o;
        int x;

        o = (obj_t *) data;
        x = player->attr.phy;

        if(!is_worn(o)) {
                player->attr.phy += o->attackmod;
        } else {
                player->attr.phy -= o->attackmod;
        }

        if(x > player->attr.phy)
                you("feel small and fragile.");
        else
                you("feel more able to perform physically challenging tasks!");
}

void oe_intelligence(void *data)
{
        obj_t *o;
        int x;

        o = (obj_t *) data;
        x = player->attr.intl;

        if(!is_worn(o)) {
                player->attr.intl += o->attackmod;
        } else {
                player->attr.intl -= o->attackmod;
        }

        if(x > player->attr.intl)
                you("feel stupider!");
        else
                you("feel smarter!");
}

void oe_dexterity(void *data)
{
        obj_t *o;
        int x;

        o = (obj_t *) data;
        x = player->attr.dex;

        if(!is_worn(o)) {
                player->attr.dex += o->attackmod;
        } else {
                player->attr.dex -= o->attackmod;
        }

        if(x > player->attr.dex)
                you("feel less agile!");
        else
                you("feel more agile!");
}

void oe_charisma(void *data)
{
        obj_t *o;
        int x;

        o = (obj_t *) data;
        x = player->attr.cha;

        if(!is_worn(o)) {
                player->attr.cha += o->attackmod;
        } else {
                player->attr.cha -= o->attackmod;
        }

        if(x > player->attr.cha)
                you("feel ugly and disgusting.");
        else
                you("feel beautiful and attractive.");
}
