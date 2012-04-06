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

typedef void (*effectfunctionpointer)(actor_t *actor, void *data);

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

void oe_heal_now(actor_t *actor, void *data)
{
        int heal;
        obj_t *o;

        o = (obj_t *) data;
        heal = dice(o->dice, o->sides, 0);
        actor->hp += heal;
        if(actor->hp > actor->maxhp)
                actor->hp = actor->maxhp;

        if(actor == player)
                youc(COLOR_INFO, "feel healed!");
}

void oe_protection_life(actor_t *actor, void *data)
{
        obj_t *o;

        o = (obj_t *)data;
        
        if(actor == player) {
                if(is_worn(o)) {
                        pr_life++;
                        youc(COLOR_INFO, "feel protected.");
                } else {
                        pr_life--;
                        youc(COLOR_INFO, "feel less protected.");
                }
        }
}

void oe_protection_fire(actor_t *actor, void *data)
{
        obj_t *o;

        o = (obj_t *)data;
        if(actor == player) {
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
}

void oe_strength(actor_t *actor, void *data)
{
        obj_t *o;
        int x;

        o = (obj_t *) data;
        x = actor->attr.str;

        if(is_potion(o))
                actor->attr.str++;
        else if(is_worn(o)) {
                actor->attr.str += o->attackmod;
        } else {
                actor->attr.str -= o->attackmod;
        }

        if(actor == player) {
                if(x > actor->attr.str)
                        youc(COLOR_INFO, "feel weaker.");
                else if(x < actor->attr.str)
                        youc(COLOR_INFO, "feel stronger!");
        }
}

void oe_wisdom(actor_t *actor, void *data)
{
        obj_t *o;
        int x;

        o = (obj_t *) data;
        x = actor->attr.wis;

        if(is_potion(o))
                actor->attr.wis++;
        else if(is_worn(o)) {
                actor->attr.wis += o->attackmod;
        } else {
                actor->attr.wis -= o->attackmod;
        }

        if(actor == player) {
                if(x > actor->attr.wis)
                        youc(COLOR_INFO, "feel more ignorant.");
                else if(x < actor->attr.wis)
                        youc(COLOR_INFO, "feel wiser!");
        }
}

void oe_physique(actor_t *actor, void *data)
{
        obj_t *o;
        int x;

        o = (obj_t *) data;
        x = actor->attr.phy;

        if(is_potion(o))
                actor->attr.phy++;
        else if(is_worn(o)) {
                actor->attr.phy += o->attackmod;
        } else {
                actor->attr.phy -= o->attackmod;
        }

        if(actor == player) {
                if(x > actor->attr.phy)
                        youc(COLOR_INFO, "feel small and fragile.");
                else if(x < actor->attr.phy)
                        youc(COLOR_INFO, "feel more able to perform physically challenging tasks!");
        }
}

void oe_intelligence(actor_t *actor, void *data)
{
        obj_t *o;
        int x;

        o = (obj_t *) data;
        x = actor->attr.intl;

        if(is_potion(o))
                actor->attr.intl++;
        else if(is_worn(o)) {
                actor->attr.intl += o->attackmod;
        } else {
                actor->attr.intl -= o->attackmod;
        }

        if(actor == player) {
                if(x > actor->attr.intl)
                        youc(COLOR_INFO, "feel stupider.");
                else if(x < actor->attr.intl)
                        youc(COLOR_INFO, "feel smarter!");
        }
}

void oe_dexterity(actor_t *actor, void *data)
{
        obj_t *o;
        int x;

        o = (obj_t *) data;
        x = actor->attr.dex;

        if(is_potion(o))
                actor->attr.dex++;
        else if(is_worn(o)) {
                actor->attr.dex += o->attackmod;
        } else {
                actor->attr.dex -= o->attackmod;
        }

        if(actor == player) {
                if(x > actor->attr.dex)
                        youc(COLOR_INFO, "feel less agile.");
                else if(x < actor->attr.dex)
                        youc(COLOR_INFO, "feel more agile!");
        }
}

void oe_charisma(actor_t *actor, void *data)
{
        obj_t *o;
        int x;

        o = (obj_t *) data;
        x = actor->attr.cha;

        if(is_potion(o))
                actor->attr.cha++;
        else if(is_worn(o)) {
                actor->attr.cha += o->attackmod;
        } else {
                actor->attr.cha -= o->attackmod;
        }

        if(actor == player) {
                if(x > actor->attr.cha)
                        youc(COLOR_INFO, "feel a bit more unattractive.");
                else if(x < actor->attr.cha)
                        youc(COLOR_INFO, "feel more attractive!");
        }
}

void apply_effect(int effect, actor_t *actor, void *data)
{
        if(!effect)
                return;

        if(effect < OE_LAST)
                effecttable[effect](actor, data);
}

void apply_effects(actor_t *actor, obj_t *o)
{
        int i;

        for(i = 0; i < o->effects; i++)
                if(o->effect[i])
                        apply_effect(o->effect[i], actor, o);
}
