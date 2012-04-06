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

#define OE_STRENGTH                1
#define OE_WISDOM                  2
#define OE_INTELLIGENCE            3
#define OE_PHYSIQUE                4
#define OE_DEXTERITY               5
#define OE_CHARISMA                6
#define OE_PROTECTION_LIFE         7
#define OE_PROTECTION_FIRE         8

#define OE_HEAL_NOW                9

#define OE_LAST                    10

void apply_effects(actor_t *actor, obj_t *o);
void apply_effect(int effect, actor_t *actor, void *data);

void oe_strength(actor_t *actor, void *data);
void oe_wisdom(actor_t *actor, void *data);
void oe_intelligence(actor_t *actor, void *data);
void oe_physique(actor_t *actor, void *data);
void oe_dexterity(actor_t *actor, void *data);
void oe_charisma(actor_t *actor, void *data);
void oe_protection_life(actor_t *actor, void *data);
void oe_protection_fire(actor_t *actor, void *data);
void oe_heal_now(actor_t *actor, void *data);

#endif
