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
#include "quest.h"
#include "npc.h"
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

int get_first_free_temp_effect_slot(actor_t *actor)
{
        int i;

        for(i=0;i<MAX_TEMP_EFFECTS;i++) {
                if(actor->effect[i] == NULL)
                        return i;
        }

        return -1;
}

void add_temporary_effect(actor_t *actor, oe_t *effect)
{
        int i;

        i = get_first_free_temp_effect_slot(actor);
        if(i < 0) {
                gtprintfc(COLOR_RED, "Your body can't handle this much!");
                return;
        }

        actor->effect[i] = effect;
        //gtprintf("Added temp effect: effect = %d, gain = %d, duration = %d, negative = %s", effect->effect, effect->gain, effect->duration, effect->negative ? "true" : "false"); 
}


void remove_temporary_effect(actor_t *actor, int i)
{
        actor->effect[i] = NULL;
}

// This one is always temporary.
void oe_invisibility(actor_t *actor, void *data, int e, bool apply)
{
        obj_t *o;

        o = (obj_t *) data;

        if(apply) {
                if(o) {
                        if(is_potion(o)) {
                                o->effect[e].duration = dice(o->effect[e].durationdice, o->effect[e].durationsides, o->effect[e].durationmodifier);
                                if(actor == player)
                                        youc(COLOR_INFO, "become invisible!");
                                else
                                        gtprintfc(COLOR_INFO, "The %s fades away and becomes invisible!", actor->name);

                                add_temporary_effect(actor, &o->effect[e]);
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
        int x;

        o = (obj_t *) data;
        x = get_strength(actor);

        if(!apply)
                actor->attrmod.str -= e;  // we use 'e' for the gain value when unapplying a temporary stat effect. makes sense, no?

        if(apply && is_potion(o)) {
                if(o->effect[e].duration == -1) {  // effect is permanent.
                        if(o->effect[e].gain)
                                actor->attr.str += o->effect[e].gain;
                        else
                                actor->attr.str++;
                } else if(o->effect[e].duration > 0) {
                        o->effect[e].duration = dice(o->effect[e].durationdice, o->effect[e].durationsides, o->effect[e].durationmodifier);
                        o->effect[e].gain = dice(o->effect[e].dice, o->effect[e].sides, o->effect[e].modifier);

                        if(o->effect[e].negative)
                                o->effect[e].gain = 0 - o->effect[e].gain;

                        actor->attrmod.str += o->effect[e].gain;

                        if(actor == player)
                                youc(COLOR_INFO, "feel a change in your body, but you sense that it may go away again soon.");

                        add_temporary_effect(actor, &o->effect[e]);
                }
        }
        
        if(apply && !is_potion(o)) {
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
        int x;

        o = (obj_t *) data;
        x = get_wisdom(actor);

        if(!apply)
                actor->attrmod.wis -= e;  // we use 'e' for the gain value when unapplying a temporary stat effect. makes sense, no?

        if(apply && is_potion(o)) {
                if(o->effect[e].duration == -1) {
                        if(o->effect[e].gain)
                                actor->attr.wis += o->effect[e].gain;
                        else
                                actor->attr.wis++;
                } else if(o->effect[e].duration > 0) {
                        o->effect[e].duration = dice(o->effect[e].durationdice, o->effect[e].durationsides, o->effect[e].durationmodifier);
                        o->effect[e].gain = dice(o->effect[e].dice, o->effect[e].sides, o->effect[e].modifier);

                        if(o->effect[e].negative)
                                o->effect[e].gain = 0 - o->effect[e].gain;
                        
                        actor->attrmod.wis += o->effect[e].gain;

                        if(actor == player)
                                youc(COLOR_INFO, "feel a change in your mind, but you sense that it may go away again soon.");

                        add_temporary_effect(actor, &o->effect[e]);
                }
        }
        
        if(apply && !is_potion(o)) {
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
        }

        if(actor == player) {
                if(x > get_wisdom(player))
                        youc(COLOR_INFO, "feel foolish!");
                else if(x < get_wisdom(player))
                        youc(COLOR_INFO, "feel wiser!");
        }
}

void oe_physique(actor_t *actor, void *data, int e, bool apply)
{
        obj_t *o;
        int x;

        o = (obj_t *) data;
        x = get_physique(actor);

        if(!apply)
                actor->attrmod.phy -= e;  // we use 'e' for the gain value when unapplying a temporary stat effect. makes sense, no?

        if(apply && is_potion(o)) {
                if(o->effect[e].duration == -1) {  // effect is permanent.
                        if(o->effect[e].gain)
                                actor->attr.phy += o->effect[e].gain;
                        else
                                actor->attr.phy++;
                } else if(o->effect[e].duration > 0) {
                        o->effect[e].duration = dice(o->effect[e].durationdice, o->effect[e].durationsides, o->effect[e].durationmodifier);
                        o->effect[e].gain = dice(o->effect[e].dice, o->effect[e].sides, o->effect[e].modifier);

                        if(o->effect[e].negative)
                                o->effect[e].gain = 0 - o->effect[e].gain;

                        actor->attrmod.phy += o->effect[e].gain;

                        if(actor == player)
                                youc(COLOR_INFO, "feel a change in your body, but you sense that it may go away again soon.");

                        add_temporary_effect(actor, &o->effect[e]);
                }
        }
        
        if(apply && !is_potion(o)) {
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
        }

        if(actor == player) {
                if(x > get_physique(player))
                        youc(COLOR_INFO, "feel smaller, more fragile.");
                else if(x < get_physique(player))
                        youc(COLOR_INFO, "feel tougher!");
        }
}

void oe_intelligence(actor_t *actor, void *data, int e, bool apply)
{
        obj_t *o;
        int x;

        o = (obj_t *) data;
        x = get_intelligence(actor);

        if(!apply)
                actor->attrmod.intl -= e;  // we use 'e' for the gain value when unapplying a temporary stat effect. makes sense, no?

        if(apply && is_potion(o)) {
                if(o->effect[e].duration == -1) {  // effect is permanent.
                        if(o->effect[e].gain)
                                actor->attr.intl += o->effect[e].gain;
                        else
                                actor->attr.intl++;
                } else if(o->effect[e].duration > 0) {
                        o->effect[e].duration = dice(o->effect[e].durationdice, o->effect[e].durationsides, o->effect[e].durationmodifier);
                        o->effect[e].gain = dice(o->effect[e].dice, o->effect[e].sides, o->effect[e].modifier);

                        if(o->effect[e].negative)
                                o->effect[e].gain = 0 - o->effect[e].gain;

                        actor->attrmod.intl += o->effect[e].gain;

                        if(actor == player)
                                youc(COLOR_INFO, "feel a change in your body, but you sense that it may go away again soon.");

                        add_temporary_effect(actor, &o->effect[e]);
                }
        }
        
        if(apply && !is_potion(o)) {
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
        }

        if(actor == player) {
                if(x > get_intelligence(player))
                        youc(COLOR_INFO, "feel stupider!");
                else if(x < get_intelligence(player))
                        youc(COLOR_INFO, "feel smarter!");
        }
}

void oe_dexterity(actor_t *actor, void *data, int e, bool apply)
{
        obj_t *o;
        int x;

        o = (obj_t *) data;
        x = get_dexterity(actor);

        if(!apply)
                actor->attrmod.dex -= e;  // we use 'e' for the gain value when unapplying a temporary stat effect. makes sense, no?

        if(apply && is_potion(o)) {
                if(o->effect[e].duration == -1) {  // effect is permanent.
                        if(o->effect[e].gain)
                                actor->attr.dex += o->effect[e].gain;
                        else
                                actor->attr.dex++;
                } else if(o->effect[e].duration > 0) {
                        o->effect[e].duration = dice(o->effect[e].durationdice, o->effect[e].durationsides, o->effect[e].durationmodifier);
                        o->effect[e].gain = dice(o->effect[e].dice, o->effect[e].sides, o->effect[e].modifier);

                        if(o->effect[e].negative)
                                o->effect[e].gain = 0 - o->effect[e].gain;

                        actor->attrmod.dex += o->effect[e].gain;

                        if(actor == player)
                                youc(COLOR_INFO, "feel a change in your body, but you sense that it may go away again soon.");

                        add_temporary_effect(actor, &o->effect[e]);
                }
        }
        
        if(apply && !is_potion(o)) {
                if(is_worn(o)) {
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
        }

        if(actor == player) {
                if(x > get_dexterity(player))
                        youc(COLOR_INFO, "feel less agile!");
                else if(x < get_dexterity(player))
                        youc(COLOR_INFO, "feel more agile!");
        }
}


void oe_charisma(actor_t *actor, void *data, int e, bool apply)
{
        obj_t *o;
        int x;

        o = (obj_t *) data;
        x = get_charisma(actor);

        if(!apply)
                actor->attrmod.cha -= e;  // we use 'e' for the gain value when unapplying a temporary stat effect. makes sense, no?

        if(apply && is_potion(o)) {
                if(o->effect[e].duration == -1) {  // effect is permanent.
                        if(o->effect[e].gain)
                                actor->attr.cha += o->effect[e].gain;
                        else
                                actor->attr.cha++;
                } else if(o->effect[e].duration > 0) {
                        o->effect[e].duration = dice(o->effect[e].durationdice, o->effect[e].durationsides, o->effect[e].durationmodifier);
                        o->effect[e].gain = dice(o->effect[e].dice, o->effect[e].sides, o->effect[e].modifier);

                        if(o->effect[e].negative)
                                o->effect[e].gain = 0 - o->effect[e].gain;

                        actor->attrmod.cha += o->effect[e].gain;

                        if(actor == player)
                                youc(COLOR_INFO, "feel a change in your body, but you sense that it may go away again soon.");

                        add_temporary_effect(actor, &o->effect[e]);
                }
        }
        
        if(apply && !is_potion(o)) {
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
        }

        if(actor == player) {
                if(x > get_charisma(player))
                        youc(COLOR_INFO, "feel less attractive!");
                else if(x < get_charisma(player))
                        youc(COLOR_INFO, "feel more attractive!");
        }
}

void apply_effect(actor_t *actor, void *data, int i)
{
        obj_t *o;
        int effect;

        o = (obj_t *) data;
        effect = o->effect[i].effect;

        if(effect < OE_LAST)
                effecttable[effect](actor, data, i, true);
}

void apply_effects(actor_t *actor, obj_t *o)
{
        int i;

        for(i = 0; i < o->effects; i++)
                if(o->effect[i].effect)
                        apply_effect(actor, o, i);
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
        int i;

        for(i = 0; i < MAX_TEMP_EFFECTS; i++) {
                if(actor->effect[i]) {
                        //gtprintf("reducing duration of effect %d (gain %d): from %d to %d", i, actor->effect[i]->gain, actor->effect[i]->duration, actor->effect[i]->duration - 1);
                        actor->effect[i]->duration--;
                        if(actor->effect[i]->duration == 0) {
                                //gtprintf("removeing effect %d.", i);
                                effecttable[actor->effect[i]->effect](actor, NULL, actor->effect[i]->gain, false);
                                remove_temporary_effect(actor, i);
                        }
                }
        }
}

// "converted" from macro to function, therefore stupid variable names
void add_effect_with_duration_dice_sides(obj_t *o, short b, short c, short d, short e, short f, short g)
{
        if(o->effects < MAX_EFFECTS) { 
                o->effect[(int)o->effects].effect = b;
                o->effect[(int)o->effects].durationdice = c;
                o->effect[(int)o->effects].durationsides = d;
                o->effect[(int)o->effects].durationmodifier = e; 
                o->effect[(int)o->effects].dice = f;
                o->effect[(int)o->effects].sides = g;
                o->effect[(int)o->effects].duration = 1;
                o->effects++;
        }
}

// "converted" from macro to function, therefore stupid variable names
void add_negative_effect_with_duration_dice_sides(obj_t *o, short b, short c, short d, short e, short f, short g)
{
        if(o->effects < MAX_EFFECTS) { 
                o->effect[(int)o->effects].effect = b;
                o->effect[(int)o->effects].durationdice = c;
                o->effect[(int)o->effects].durationsides = d;
                o->effect[(int)o->effects].durationmodifier = e; 
                o->effect[(int)o->effects].dice = f;
                o->effect[(int)o->effects].sides = g;
                o->effect[(int)o->effects].duration = 1;
                o->effect[(int)o->effects].negative = true;
                o->effects++;
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
