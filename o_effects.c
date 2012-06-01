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

typedef void (*effectfunctionpointer)(actor_t *actor, void *data, int e, bool apply);

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

void schedule_temporary_effect(actor_t *actor, int duration, int effect, int action, obj_t *object, int gain)
{
        int i, j;

        for(i = 0; i < (duration*10); i += 10) {
                j = schedule_action_delayed(action, actor, object, i);
                act[j].gain = gain;
        }

        actor->temp[effect] += duration;
}

// This one is always temporary.
void oe_invisibility(actor_t *actor, void *data, int e, bool apply)
{
        obj_t *o;
        int duration;

        o = (obj_t *) data;

        if(apply) {
                if(o) {
                        if(is_potion(o)) {
                                duration = dice(o->effect[e].durationdice, o->effect[e].durationsides, o->effect[e].durationmodifier);
                                if(actor == player)
                                        youc(COLOR_INFO, "become invisible!");
                                else
                                        gtprintfc(COLOR_INFO, "The %s fades away and becomes invisible!", actor->name);

                                schedule_temporary_effect(actor, duration, TEMP_INVISIBLE, ACTION_DECREASE_INVISIBILITY, o, 0);
                                setbit(actor->flags, MF_INVISIBLE);
                        }
                }
        } else {
                if(actor == player)
                        gtprintfc(COLOR_INFO, "You become visible again.");
                else
                        gtprintfc(COLOR_INFO, "The %s becomes visible again.", actor->name);

                clearbit(actor->flags, MF_INVISIBLE);
        }
}

void oe_heal_now(actor_t *actor, void *data, int e, bool apply)
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

void oe_speed(actor_t *actor, void *data, int e, bool apply)
{
        obj_t *o;

        o = (obj_t *)data;
        
        gtprintf("speedeffect!");
        if(actor == player) {
                if(is_worn(o)) {
                        gtprintf("speed = %d   gain = %d", player->speed, o->effect[e].gain);
                        player->speed -= o->effect[e].gain;
                        gtprintf("speed = %d   gain = %d", player->speed, o->effect[e].gain);
                        youc(COLOR_INFO, "feel faster!");
                } else {
                        player->speed += o->effect[e].gain;
                        youc(COLOR_INFO, "feel slower.");
                }
        }

}

void oe_protection_life(actor_t *actor, void *data, int e, bool apply)
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

void oe_protection_fire(actor_t *actor, void *data, int e, bool apply)
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

void oe_strength(actor_t *actor, void *data, int e, bool apply)
{
        obj_t *o;
        int x, gain, duration;

        o = (obj_t *) data;
        x = get_strength(actor);

        if(!apply)
                actor->attrmod.str -= e;  // we use 'e' for the gain value when unapplying a temporary stat effect. makes sense, no?

        if(apply && is_potion(o)) {
                if(o->effect[e].duration == -1) {
                        if(o->effect[e].gain)
                                actor->attr.str += o->effect[e].gain;
                        else
                                actor->attr.str++;
                } else if(o->effect[e].duration > 0) {
                        duration = dice(o->effect[e].durationdice, o->effect[e].durationsides, o->effect[e].durationmodifier);
                        gain = dice(o->effect[e].dice, o->effect[e].sides, o->effect[e].modifier);

                        actor->attrmod.str += gain;

                        if(actor == player)
                                youc(COLOR_INFO, "feel a change in your body, but you sense that it may go away again soon.");

                        schedule_temporary_effect(actor, duration, TEMP_STRENGTH, ACTION_DECREASE_TEMP_STRENGTH, o, gain);
                }
        }
        
        if(apply) {
                if(is_worn(o)) {
                        if(o->effect[e].gain)
                                actor->attrmod.str += o->effect[e].gain;
                        else
                                actor->attrmod.str += o->attackmod;
                } else {
                        if(o->effect[e].gain)
                                actor->attrmod.str -= o->effect[e].gain;
                        else
                                actor->attrmod.str -= o->attackmod;
                }
        }

        if(actor == player) {
                if(x > get_strength(player))
                        youc(COLOR_INFO, "feel weaker.");
                else if(x < get_strength(player))
                        youc(COLOR_INFO, "feel stronger!");
        }
}

void oe_wisdom(actor_t *actor, void *data, int e, bool apply)
{
        obj_t *o;
        int x, gain, duration;

        o = (obj_t *) data;
        x = get_wisdom(actor);

        if(is_potion(o)) {
                if(o->effect[e].duration == -1) {
                        actor->attr.wis++;
                } else if(o->effect[e].duration > 0) {
                        duration = dice(o->effect[e].durationdice, o->effect[e].durationsides, o->effect[e].durationmodifier);
                        if(o->effect[e].gain)
                                gain = o->effect[e].gain;
                        else
                                gain = dice(o->effect[e].dice, o->effect[e].sides, o->effect[e].modifier);
                        actor->attrmod.wis += gain;
                        if(actor == player)
                                youc(COLOR_INFO, "feel a change in your mind, but you sense that it may go away again soon.");

                        schedule_temporary_effect(actor, duration, TEMP_WISDOM, ACTION_DECREASE_TEMP_WISDOM, o, gain);
                }
        }

        if(is_worn(o)) {
                if(o->effect[e].gain)
                        actor->attrmod.wis += o->effect[e].gain;
                else
                        actor->attrmod.wis += o->attackmod;
        } else {
                if(o->effect[e].gain)
                        actor->attrmod.wis -= o->effect[e].gain;
                else
                        actor->attrmod.wis -= o->attackmod;
        }

        if(actor == player) {
                if(x > get_wisdom(player))
                        youc(COLOR_INFO, "feel more ignorant.");
                else if(x < get_wisdom(player))
                        youc(COLOR_INFO, "feel wiser!");
        }
}

void oe_physique(actor_t *actor, void *data, int e, bool apply)
{
        obj_t *o;
        int x, gain, duration;

        o = (obj_t *) data;
        x = get_physique(actor);

        if(is_potion(o)) {
                if(o->effect[e].duration == -1) {
                        actor->attr.phy++;
                } else if(o->effect[e].duration > 0) {
                        duration = dice(o->effect[e].durationdice, o->effect[e].durationsides, o->effect[e].durationmodifier);
                        if(o->effect[e].gain)
                                gain = o->effect[e].gain;
                        else
                                gain = dice(o->effect[e].dice, o->effect[e].sides, o->effect[e].modifier);
                        actor->attrmod.phy += gain;
                        if(actor == player)
                                youc(COLOR_INFO, "feel a change in your mind, but you sense that it may go away again soon.");

                        schedule_temporary_effect(actor, duration, TEMP_PHYSIQUE, ACTION_DECREASE_TEMP_PHYSIQUE, o, gain);
                }
        }

        if(is_worn(o)) {
                if(o->effect[e].gain)
                        actor->attrmod.phy += o->effect[e].gain;
                else
                        actor->attrmod.phy += o->attackmod;
        } else {
                if(o->effect[e].gain)
                        actor->attrmod.phy -= o->effect[e].gain;
                else
                        actor->attrmod.phy -= o->attackmod;
        }

        if(actor == player) {
                if(x > get_physique(player))
                        youc(COLOR_INFO, "feel small and fragile.");
                else if(x < get_physique(player))
                        youc(COLOR_INFO, "feel more able to perform physically challenging tasks!");
        }
}

void oe_intelligence(actor_t *actor, void *data, int e, bool apply)
{
        obj_t *o;
        int x, gain, duration;

        o = (obj_t *) data;
        x = get_intelligence(actor);

        if(is_potion(o)) {
                if(o->effect[e].duration == -1) {
                        actor->attr.intl++;
                } else if(o->effect[e].duration > 0) {
                        duration = dice(o->effect[e].durationdice, o->effect[e].durationsides, o->effect[e].durationmodifier);
                        if(o->effect[e].gain)
                                gain = o->effect[e].gain;
                        else
                                gain = dice(o->effect[e].dice, o->effect[e].sides, o->effect[e].modifier);
                        actor->attrmod.intl += gain;
                        if(actor == player)
                                youc(COLOR_INFO, "feel a change in your mind, but you sense that it may go away again soon.");

                        schedule_temporary_effect(actor, duration, TEMP_INTELLIGENCE, ACTION_DECREASE_TEMP_INTELLIGENCE, o, gain);
                }
        }

        if(is_worn(o)) {
                if(o->effect[e].gain)
                        actor->attrmod.intl += o->effect[e].gain;
                else
                        actor->attrmod.intl += o->attackmod;
        } else {
                if(o->effect[e].gain)
                        actor->attrmod.intl -= o->effect[e].gain;
                else
                        actor->attrmod.intl -= o->attackmod;
        }

        if(actor == player) {
                if(x > get_intelligence(player))
                        youc(COLOR_INFO, "feel stupider.");
                else if(x < get_intelligence(player))
                        youc(COLOR_INFO, "feel smarter!");
        }
}

void oe_dexterity(actor_t *actor, void *data, int e, bool apply)
{
        obj_t *o;
        int x, gain, duration;

        o = (obj_t *) data;
        x = get_dexterity(actor);

        if(is_potion(o)) {
                if(o->effect[e].duration == -1) {
                        actor->attr.dex++;
                } else if(o->effect[e].duration > 0) {
                        duration = dice(o->effect[e].durationdice, o->effect[e].durationsides, o->effect[e].durationmodifier);
                        if(o->effect[e].gain)
                                gain = o->effect[e].gain;
                        else
                                gain = dice(o->effect[e].dice, o->effect[e].sides, o->effect[e].modifier);
                        actor->attrmod.dex += gain;
                        if(actor == player)
                                youc(COLOR_INFO, "feel a change in your body, but you sense that it may go away again soon.");

                        schedule_temporary_effect(actor, duration, TEMP_DEXTERITY, ACTION_DECREASE_TEMP_DEXTERITY, o, gain);
                }
        } else if(is_worn(o)) {
                if(o->effect[e].gain)
                        actor->attrmod.dex += o->effect[e].gain;
                else
                        actor->attrmod.dex += o->attackmod;
        } else {
                if(o->effect[e].gain)
                        actor->attrmod.dex -= o->effect[e].gain;
                else
                        actor->attrmod.dex -= o->attackmod;
        }

        if(actor == player) {
                if(x > get_dexterity(player))
                        youc(COLOR_INFO, "feel less agile.");
                else if(x < get_dexterity(player))
                        youc(COLOR_INFO, "feel more agile!");
        }
}

void oe_charisma(actor_t *actor, void *data, int e, bool apply)
{
        obj_t *o;
        int x, gain, duration;

        o = (obj_t *) data;
        x = get_charisma(actor);

        if(is_potion(o)) {
                if(o->effect[e].duration == -1) {
                        actor->attr.cha++;
                } else if(o->effect[e].duration > 0) {
                        duration = dice(o->effect[e].durationdice, o->effect[e].durationsides, o->effect[e].durationmodifier);
                        if(o->effect[e].gain)
                                gain = o->effect[e].gain;
                        else
                                gain = dice(o->effect[e].dice, o->effect[e].sides, o->effect[e].modifier);
                        actor->attrmod.cha += gain;
                        if(actor == player)
                                youc(COLOR_INFO, "feel a change in your body, but you sense that it may go away again soon.");
                        
                        // TODO/IDEA: separate attr for attrgain!
                        schedule_temporary_effect(actor, duration, TEMP_CHARISMA, ACTION_DECREASE_TEMP_CHARISMA, o, gain);
                }
        }

        if(is_worn(o)) {
                if(o->effect[e].gain)
                        actor->attrmod.cha += o->effect[e].gain;
                else
                        actor->attrmod.cha += o->attackmod;
        } else {
                if(o->effect[e].gain)
                        actor->attrmod.cha -= o->effect[e].gain;
                else
                        actor->attrmod.cha -= o->attackmod;
        }

        if(actor == player) {
                if(x > get_charisma(player))
                        youc(COLOR_INFO, "feel less attractive.");
                else if(x < get_charisma(player))
                        youc(COLOR_INFO, "feel more attractive!");
        }
}

void apply_effect(int e, int effect, actor_t *actor, void *data)
{
        if(!effect)
                return;

        if(effect < OE_LAST)
                effecttable[effect](actor, data, e, true);
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
