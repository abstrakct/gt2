/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Object effects
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef O_EFFECTS_H
#define O_EFFECTS_H

#include "actor.h"

#define add_effect(a, b) if(a->effects < MAX_EFFECTS) { a->effect[(int)a->effects].effect = b; a->effects++; }
#define add_effect_with_duration(a, b, c) if(a->effects < MAX_EFFECTS) { a->effect[(int)a->effects].effect = b; a->effect[(int)a->effects].duration = c; a->effects++; }

#define OE_STRENGTH                1
#define OE_WISDOM                  2
#define OE_INTELLIGENCE            3
#define OE_PHYSIQUE                4
#define OE_DEXTERITY               5
#define OE_CHARISMA                6
#define OE_SPEED                   7

#define OE_PROTECTION_LIFE         20 
#define OE_PROTECTION_FIRE         21

#define OE_HEAL_NOW                30
#define OE_INVISIBILITY            31

#define OE_LAST                    32

void apply_effects(actor_t *actor, obj_t *o);
void apply_effect(int e, int effect, actor_t *actor, void *data);
void process_temp_effects(actor_t *actor);

void oe_strength(actor_t *actor, void *data, int e);
void oe_wisdom(actor_t *actor, void *data, int e);
void oe_intelligence(actor_t *actor, void *data, int e);
void oe_physique(actor_t *actor, void *data, int e);
void oe_dexterity(actor_t *actor, void *data, int e);
void oe_charisma(actor_t *actor, void *data, int e);
void oe_protection_life(actor_t *actor, void *data, int e);
void oe_protection_fire(actor_t *actor, void *data, int e);
void oe_heal_now(actor_t *actor, void *data, int e);
void oe_invisibility(actor_t *actor, void *data, int e);
void oe_speed(actor_t *actor, void *data, int e);


void add_effect_with_duration_dice_sides(obj_t *a, short b, short c, short d, short e, short f, short g);
void add_effect_with_duration_and_floatgain(obj_t *a, int effect, int duration, float gain);
void add_effect_with_duration_and_intgain(obj_t *a, int effect, int duration, int gain);
#endif
