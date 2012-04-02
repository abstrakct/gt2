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
#include "world.h"
#include "datafiles.h"
#include "display.h"
#include "debug.h"
#include "gt.h"

typedef void (*effectfunctionpointer)(void *data);

/* TODO: actor!!! */

effectfunctionpointer effecttable[] = {
        0,
        oe_strength,
        oe_wisdom,
        oe_intelligence,
        oe_physique,
        oe_dexterity,
        oe_charisma,
        oe_protection_life,
        oe_protection_fire,
        oe_heal_now
};

void oe_heal_now(void *data)
{
        int heal;
        obj_t *o;

        o = (obj_t *) data;
        heal = dice(o->dice, o->sides, 0);
        player->hp += heal;
        if(player->hp > player->maxhp)
                player->hp = player->maxhp;

        youc(COLOR_INFO, "feel healed!");
}

void oe_protection_life(void *data)
{
        obj_t *o;

        o = (obj_t *)data;
        if(is_worn(o)) {
                pr_life++;
                youc(COLOR_INFO, "feel protected.");
        } else {
                pr_life--;
                youc(COLOR_INFO, "feel less protected.");
        }
}

void oe_protection_fire(void *data)
{
        obj_t *o;

        o = (obj_t *)data;
        if(is_worn(o))
                pr_fire++;
        else
                pr_fire--;

        switch(pr_fire) {
                case 1:
                        youc(COLOR_INFO, "feel a bit resistant to fire.");
                        break;
                case 2:
                        youc(COLOR_INFO, "feel rather resistant to fire.");
                        break;
                case 3:
                        youc(COLOR_INFO, "feel quite resistant to fire.");
                        break;
                case 4:
                        pr_fire = 3;
                        break;
        }
}

void oe_strength(void *data)
{
        obj_t *o;
        int x;

        o = (obj_t *) data;
        x = player->attr.str;

        if(is_potion(o))
                player->attr.str++;
        else if(is_worn(o)) {
                player->attr.str += o->attackmod;
        } else {
                player->attr.str -= o->attackmod;
        }

        if(x > player->attr.str)
                youc(COLOR_INFO, "feel weaker.");
        else if(x < player->attr.str)
                youc(COLOR_INFO, "feel stronger!");
}

void oe_wisdom(void *data)
{
        obj_t *o;
        int x;

        o = (obj_t *) data;
        x = player->attr.wis;

        if(is_potion(o))
                player->attr.wis++;
        else if(is_worn(o)) {
                player->attr.wis += o->attackmod;
        } else {
                player->attr.wis -= o->attackmod;
        }

        if(x > player->attr.wis)
                youc(COLOR_INFO, "feel more ignorant.");
        else if(x < player->attr.wis)
                youc(COLOR_INFO, "feel wiser!");
}

void oe_physique(void *data)
{
        obj_t *o;
        int x;

        o = (obj_t *) data;
        x = player->attr.phy;

        if(is_potion(o))
                player->attr.phy++;
        else if(is_worn(o)) {
                player->attr.phy += o->attackmod;
        } else {
                player->attr.phy -= o->attackmod;
        }

        if(x > player->attr.phy)
                youc(COLOR_INFO, "feel small and fragile.");
        else if(x < player->attr.phy)
                youc(COLOR_INFO, "feel more able to perform physically challenging tasks!");
}

void oe_intelligence(void *data)
{
        obj_t *o;
        int x;

        o = (obj_t *) data;
        x = player->attr.intl;

        if(is_potion(o))
                player->attr.intl++;
        else if(is_worn(o)) {
                player->attr.intl += o->attackmod;
        } else {
                player->attr.intl -= o->attackmod;
        }

        if(x > player->attr.intl)
                youc(COLOR_INFO, "feel stupider.");
        else if(x < player->attr.intl)
                youc(COLOR_INFO, "feel smarter!");
}

void oe_dexterity(void *data)
{
        obj_t *o;
        int x;

        o = (obj_t *) data;
        x = player->attr.dex;

        if(is_potion(o))
                player->attr.dex++;
        else if(is_worn(o)) {
                player->attr.dex += o->attackmod;
        } else {
                player->attr.dex -= o->attackmod;
        }

        if(x > player->attr.dex)
                youc(COLOR_INFO, "feel less agile.");
        else if(x < player->attr.dex)
                youc(COLOR_INFO, "feel more agile!");
}

void oe_charisma(void *data)
{
        obj_t *o;
        int x;

        o = (obj_t *) data;
        x = player->attr.cha;

        if(is_potion(o))
                player->attr.cha++;
        else if(is_worn(o)) {
                player->attr.cha += o->attackmod;
        } else {
                player->attr.cha -= o->attackmod;
        }

        if(x > player->attr.cha)
                youc(COLOR_INFO, "feel a bit more unattractive.");
        else if(x < player->attr.cha)
                youc(COLOR_INFO, "feel more attractive!");
}

void apply_effect(int effect, void *data)
{
        if(!effect)
                return;

        if(effect < OE_LAST)
                effecttable[effect](data);
}

void apply_effects(obj_t *o)
{
        int i;

        for(i = 0; i < o->effects; i++)
                if(o->effect[i])
                        apply_effect(o->effect[i], o);
}
