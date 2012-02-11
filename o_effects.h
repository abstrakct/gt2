/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Object effects
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef O_EFFECTS_H
#define O_EFFECTS_H

#define OE_STRENGTH     1
#define OE_WISDOM       2
#define OE_INTELLIGENCE 3
#define OE_PHYSIQUE     4
#define OE_DEXTERITY    5
#define OE_CHARISMA     6
#define OE_LAST         7

void apply_effects(obj_t *o);
void apply_effect(int effect, void *data);

void oe_strength(void *data);
void oe_wisdom(void *data);
void oe_intelligence(void *data);
void oe_physique(void *data);
void oe_dexterity(void *data);
void oe_charisma(void *data);

#endif
