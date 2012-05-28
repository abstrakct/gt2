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
#include "world.h"
#include "datafiles.h"
#include "io.h"
#include "debug.h"
#include "gt.h"
#include "utils.h"

typedef void (*effectfunctionpointer)(actor_t *actor, void *data, int e);

effectfunctionpointer effecttable[] = {
        0,
        oe_strength,
        oe_wisdom,
        oe_intelligence,
        oe_physique,
        oe_dexterity,
        oe_charisma,
        oe_speed,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        oe_protection_life,
        oe_protection_fire,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        oe_heal_now,
        oe_invisibility
};

void schedule_temporary_effect(actor_t *actor, int duration, int effect, int action)
{
        int i;
        for(i = 0; i < (duration*10); i += 10)
                schedule_action_delayed(action, actor, i);

        actor->temp[effect] += duration;
}

// This one is always temporary.
void oe_invisibility(actor_t *actor, void *data, int e)
{
        obj_t *o;
        int duration;

        o = (obj_t *) data;

        if(is_potion(o)) {
                duration = dice(o->effect[e].durationdice, o->effect[e].durationsides, o->effect[e].durationmodifier);
                if(actor == player)
                        youc(COLOR_INFO, "become invisible!");
                /*actor->temp = gtmalloc(sizeof(oe_t));
                *(actor->temp) = o->effect[e];
                actor->temp->duration = duration;*/

                schedule_temporary_effect(actor, duration, TEMP_INVISIBLE, ACTION_DECREASE_INVISIBILITY);
                setbit(actor->flags, MF_INVISIBLE);
        }
}

void oe_heal_now(actor_t *actor, void *data, int e)
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

void oe_speed(actor_t *actor, void *data, int e)
{
        obj_t *o;

        o = (obj_t *)data;
        
        gtprintf("speedeffect!");
        if(actor == player) {
                if(is_worn(o)) {
                        gtprintf("speed = %.2f   fgain = %.2f", player->speed, o->effect[e].fgain);
                        player->speed += o->effect[e].fgain;
                        gtprintf("speed = %.2f   fgain = %.2f", player->speed, o->effect[e].fgain);
                        youc(COLOR_INFO, "feel faster!");
                } else {
                        player->speed -= o->effect[e].fgain;
                        youc(COLOR_INFO, "feel slower.");
                }
        }

}

void oe_protection_life(actor_t *actor, void *data, int e)
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

void oe_protection_fire(actor_t *actor, void *data, int e)
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

void oe_strength(actor_t *actor, void *data, int e)
{
        obj_t *o;
        int x, gain, duration;

        o = (obj_t *) data;
        x = actor->attr.str;

        if(is_potion(o)) {
                if(o->effect[e].duration == -1) {
                        if(o->effect[e].gain)
                                actor->attr.str += o->effect[e].gain;
                        else
                                actor->attr.str++;
                } else if(o->effect[e].duration > 0) {
                        duration = dice(o->effect[e].durationdice, o->effect[e].durationsides, o->effect[e].durationmodifier);
                        gain = dice(o->effect[e].dice, o->effect[e].sides, o->effect[e].modifier);
                        actor->attr.str += gain;
                        if(actor == player)
                                youc(COLOR_INFO, "feel a change in your body, but you sense that it may go away again soon.");

                        schedule_temporary_effect(actor, duration, TEMP_STRENGTH, ACTION_DECREASE_TEMP_STRENGTH);
                }
        }
        
        if(is_worn(o)) {
                if(o->effect[e].gain)
                        actor->attr.str += o->effect[e].gain;
                else
                        actor->attr.str += o->attackmod;
        } else {
                if(o->effect[e].gain)
                        actor->attr.str -= o->effect[e].gain;
                else
                        actor->attr.str -= o->attackmod;
        }

        if(actor == player) {
                if(x > actor->attr.str)
                        youc(COLOR_INFO, "feel weaker.");
                else if(x < actor->attr.str)
                        youc(COLOR_INFO, "feel stronger!");
        }
}

void oe_wisdom(actor_t *actor, void *data, int e)
{
        obj_t *o;
        int x, gain, duration;

        o = (obj_t *) data;
        x = actor->attr.wis;

        if(is_potion(o)) {
                if(o->effect[e].duration == -1) {
                        actor->attr.wis++;
                } else if(o->effect[e].duration > 0) {
                        duration = dice(o->effect[e].durationdice, o->effect[e].durationsides, o->effect[e].durationmodifier);
                        if(o->effect[e].gain)
                                gain = o->effect[e].gain;
                        else
                                gain = dice(o->effect[e].dice, o->effect[e].sides, o->effect[e].modifier);
                        actor->attr.wis += gain;
                        if(actor == player)
                                youc(COLOR_INFO, "feel a change in your mind, but you sense that it may go away again soon.");

                        schedule_temporary_effect(actor, duration, TEMP_WISDOM, ACTION_DECREASE_TEMP_WISDOM);
                }
        }

        if(is_worn(o)) {
                if(o->effect[e].gain)
                        actor->attr.wis += o->effect[e].gain;
                else
                        actor->attr.wis += o->attackmod;
        } else {
                if(o->effect[e].gain)
                        actor->attr.wis -= o->effect[e].gain;
                else
                        actor->attr.wis -= o->attackmod;
        }

        if(actor == player) {
                if(x > actor->attr.wis)
                        youc(COLOR_INFO, "feel more ignorant.");
                else if(x < actor->attr.wis)
                        youc(COLOR_INFO, "feel wiser!");
        }
}

void oe_physique(actor_t *actor, void *data, int e)
{
        obj_t *o;
        int x, gain, duration;

        o = (obj_t *) data;
        x = actor->attr.phy;

        if(is_potion(o)) {
                if(o->effect[e].duration == -1) {
                        actor->attr.phy++;
                } else if(o->effect[e].duration > 0) {
                        duration = dice(o->effect[e].durationdice, o->effect[e].durationsides, o->effect[e].durationmodifier);
                        if(o->effect[e].gain)
                                gain = o->effect[e].gain;
                        else
                                gain = dice(o->effect[e].dice, o->effect[e].sides, o->effect[e].modifier);
                        actor->attr.phy += gain;
                        if(actor == player)
                                youc(COLOR_INFO, "feel a change in your mind, but you sense that it may go away again soon.");

                        schedule_temporary_effect(actor, duration, TEMP_PHYSIQUE, ACTION_DECREASE_TEMP_PHYSIQUE);
                }
        }

        if(is_worn(o)) {
                if(o->effect[e].gain)
                        actor->attr.phy += o->effect[e].gain;
                else
                        actor->attr.phy += o->attackmod;
        } else {
                if(o->effect[e].gain)
                        actor->attr.phy -= o->effect[e].gain;
                else
                        actor->attr.phy -= o->attackmod;
        }

        if(actor == player) {
                if(x > actor->attr.phy)
                        youc(COLOR_INFO, "feel small and fragile.");
                else if(x < actor->attr.phy)
                        youc(COLOR_INFO, "feel more able to perform physically challenging tasks!");
        }
}

void oe_intelligence(actor_t *actor, void *data, int e)
{
        obj_t *o;
        int x, gain, duration;

        o = (obj_t *) data;
        x = actor->attr.intl;

        if(is_potion(o)) {
                if(o->effect[e].duration == -1) {
                        actor->attr.intl++;
                } else if(o->effect[e].duration > 0) {
                        duration = dice(o->effect[e].durationdice, o->effect[e].durationsides, o->effect[e].durationmodifier);
                        if(o->effect[e].gain)
                                gain = o->effect[e].gain;
                        else
                                gain = dice(o->effect[e].dice, o->effect[e].sides, o->effect[e].modifier);
                        actor->attr.intl += gain;
                        if(actor == player)
                                youc(COLOR_INFO, "feel a change in your mind, but you sense that it may go away again soon.");

                        schedule_temporary_effect(actor, duration, TEMP_INTELLIGENCE, ACTION_DECREASE_TEMP_INTELLIGENCE);
                }
        }

        if(is_worn(o)) {
                if(o->effect[e].gain)
                        actor->attr.intl += o->effect[e].gain;
                else
                        actor->attr.intl += o->attackmod;
        } else {
                if(o->effect[e].gain)
                        actor->attr.intl -= o->effect[e].gain;
                else
                        actor->attr.intl -= o->attackmod;
        }

        if(actor == player) {
                if(x > actor->attr.intl)
                        youc(COLOR_INFO, "feel stupider.");
                else if(x < actor->attr.intl)
                        youc(COLOR_INFO, "feel smarter!");
        }
}

void oe_dexterity(actor_t *actor, void *data, int e)
{
        obj_t *o;
        int x, gain, duration;

        o = (obj_t *) data;
        x = actor->attr.dex;

        if(is_potion(o)) {
                if(o->effect[e].duration == -1) {
                        actor->attr.dex++;
                } else if(o->effect[e].duration > 0) {
                        duration = dice(o->effect[e].durationdice, o->effect[e].durationsides, o->effect[e].durationmodifier);
                        if(o->effect[e].gain)
                                gain = o->effect[e].gain;
                        else
                                gain = dice(o->effect[e].dice, o->effect[e].sides, o->effect[e].modifier);
                        actor->attr.dex += gain;
                        if(actor == player)
                                youc(COLOR_INFO, "feel a change in your body, but you sense that it may go away again soon.");

                        schedule_temporary_effect(actor, duration, TEMP_DEXTERITY, ACTION_DECREASE_TEMP_DEXTERITY);
                }
        } else if(is_worn(o)) {
                if(o->effect[e].gain)
                        actor->attr.dex += o->effect[e].gain;
                else
                        actor->attr.dex += o->attackmod;
        } else {
                if(o->effect[e].gain)
                        actor->attr.dex -= o->effect[e].gain;
                else
                        actor->attr.dex -= o->attackmod;
        }

        if(actor == player) {
                if(x > actor->attr.dex)
                        youc(COLOR_INFO, "feel less agile.");
                else if(x < actor->attr.dex)
                        youc(COLOR_INFO, "feel more agile!");
        }
}

void oe_charisma(actor_t *actor, void *data, int e)
{
        obj_t *o;
        int x, gain, duration;

        o = (obj_t *) data;
        x = actor->attr.cha;

        if(is_potion(o)) {
                if(o->effect[e].duration == -1) {
                        actor->attr.cha++;
                } else if(o->effect[e].duration > 0) {
                        duration = dice(o->effect[e].durationdice, o->effect[e].durationsides, o->effect[e].durationmodifier);
                        if(o->effect[e].gain)
                                gain = o->effect[e].gain;
                        else
                                gain = dice(o->effect[e].dice, o->effect[e].sides, o->effect[e].modifier);
                        actor->attr.cha += gain;
                        if(actor == player)
                                youc(COLOR_INFO, "feel a change in your body, but you sense that it may go away again soon.");
                        
                        // TODO/IDEA: separate attr for attrgain!
                        schedule_temporary_effect(actor, duration, TEMP_CHARISMA, ACTION_DECREASE_TEMP_CHARISMA);
                }
        }

        if(is_worn(o)) {
                if(o->effect[e].gain)
                        actor->attr.cha += o->effect[e].gain;
                else
                        actor->attr.cha += o->attackmod;
        } else {
                if(o->effect[e].gain)
                        actor->attr.cha -= o->effect[e].gain;
                else
                        actor->attr.cha -= o->attackmod;
        }

        if(actor == player) {
                if(x > actor->attr.cha)
                        youc(COLOR_INFO, "feel less attractive.");
                else if(x < actor->attr.cha)
                        youc(COLOR_INFO, "feel more attractive!");
        }
}

void apply_effect(int e, int effect, actor_t *actor, void *data)
{
        if(!effect)
                return;

        if(effect < OE_LAST)
                effecttable[effect](actor, data, e);
}

void apply_effects(actor_t *actor, obj_t *o)
{
        int i;

        for(i = 0; i < o->effects; i++)
                if(o->effect[i].effect)
                        apply_effect(i, o->effect[i].effect, actor, o);
}

void unapply_temp_effect(actor_t *actor)
{
        /*int old;

        switch(actor->temp->effect) {
                case OE_STRENGTH:
                        old = actor->attr.str;
                        actor->attr.str -= actor->temp->gain;
                        if(actor == player)
                                youc(COLOR_INFO, "feel %s again.", (old > actor->attr.str ? "weaker" : "stronger"));
                        gtfree(actor->temp);
                        actor->temp = NULL;
                        break;
                case OE_INVISIBILITY:
                        if(actor == player)
                                gtprintfc(COLOR_INFO, "Your body looks more solid again.");
                        clearbit(actor->flags, MF_INVISIBLE);
                        gtfree(actor->temp);
                        actor->temp = NULL;
                        break;
                default:
                        gtprintf("trying to unapply unknown effect %d.", actor->temp->effect);
                        break;
        }*/
}

void process_temp_effects(actor_t *actor)
{
        /*actor->temp->duration--;
        if(actor->temp->duration == 0)
                unapply_temp_effect(actor);*/
}

// "converted" from macro to function, therefore stupid variable names
void add_effect_with_duration_dice_sides(obj_t *a, short b, short c, short d, short e, short f, short g)
{
        if(a->effects < MAX_EFFECTS) { 
                a->effect[(int)a->effects].effect = b;
                a->effect[(int)a->effects].durationdice = c;
                a->effect[(int)a->effects].durationsides = d;
                a->effect[(int)a->effects].durationmodifier = e; 
                a->effect[(int)a->effects].dice = f;
                a->effect[(int)a->effects].sides = g;
                a->effect[(int)a->effects].duration = 1;
                a->effects++;
        }
}

void add_effect_with_duration_and_floatgain(obj_t *a, int effect, int duration, float gain)
{
        if(a->effects < MAX_EFFECTS) { 
                a->effect[(int)a->effects].effect   = effect;
                a->effect[(int)a->effects].duration = duration;
                a->effect[(int)a->effects].fgain    = gain;
                a->effects++;
        }
}

void add_effect_with_duration_and_intgain(obj_t *a, int effect, int duration, int gain)
{
        if(a->effects < MAX_EFFECTS) { 
                a->effect[(int)a->effects].effect   = effect;
                a->effect[(int)a->effects].duration = duration;
                a->effect[(int)a->effects].gain     = gain;
                a->effects++;
        }
}
