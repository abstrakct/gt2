/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * This file deals with general creature/actor stuff
 *
 * Copyright 2011 Rolf Klausen
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "objects.h"
#include "actor.h"
#include "monsters.h"
#include "utils.h"
#include "datafiles.h"
#include "world.h"
#include "display.h"
#include "gt.h"

obj_t *objlet[52];    // 52 pointers to objects, a-z & A-Z

#define SGN(a) (((a)<0) ? -1 : 1)

//                            x   1   2   3   4   5   6   7   8  9 10 11 12 13 14 15 16 17 18 19 20
//int strength_modifier[20] = { 0, -5, -4, -3, -3, -2, -2, -1, -1, 0, 0, 0, 0, 0, 1, 2, 2, 3, 3, 4, 5 };



// object-to-letter and vise versa 

char get_first_free_letter()
{
        int i;

        for(i = 0; i < 52; i++) {
                if(!objlet[i])
                        return(slot_to_letter(i));
        }
        
        return 0;    // == no free slots!
}

char slot_to_letter(int i)
{
        int retval;

        retval = i;
        if(i >= 0 && i <= 25)
                retval += 97;
        if(i >= 26 && i < 52)
                retval += 39;

        return retval;
}

int letter_to_slot(char c)
{
        int retval;

        retval = c;
        if(c >= 97 && c <= 122)
                retval -= 97;
        if(c >= 65 && c <= 90)
                retval -= 39;

        return retval;
}

obj_t *get_object_from_letter(char c)
{
        return objlet[letter_to_slot(c)];
}

int object_to_slot(obj_t *o)
{
        int i;

        for(i = 0; i < 52; i++) {
                if(objlet[i] == o)
                        return i;
        }

        return -1;
}

void assign_free_slot(obj_t *o)
{
        char c;

        c = get_first_free_letter();
        o->slot = c;
        objlet[letter_to_slot(c)] = o;
}

void unassign_object(obj_t *o)
{
        objlet[object_to_slot(o)] = NULL;
        o->slot = 0;
}

void assign_letter(char c, obj_t *o)
{
        objlet[letter_to_slot(c)] = o;
}

bool actor_in_lineofsight(actor_t *src, actor_t *dest)
{
        return in_lineofsight(src, dest->y, dest->x);
}

/*
 * This function will check if the actor src can see cell at goaly,goalx
 * Returns true if it can, false if not,
 *
 * Adapted from http://roguebasin.roguelikedevelopment.org/index.php/Simple_Line_of_Sight
 */
bool in_lineofsight(actor_t *src, int goaly, int goalx)
{
        int t, x, y, ax, ay, sx, sy, dx, dy;

        if(src->x == goalx && src->y == goaly) // shouldn't actually happen?
                return true;

        dx = goalx - src->x;
        dy = goaly - src->y;

        ax = abs(dx) << 1;
        ay = abs(dy) << 1;

        sx = SGN(dx);
        sy = SGN(dy);

        x = src->x;
        y = src->y;

        // This must be changed to a FOV thing!
        if(goalx > (x + src->viewradius))
                return false;
        if(goaly > (y + src->viewradius))
                return false;
        if(goalx < (x - src->viewradius))
                return false;
        if(goaly < (y - src->viewradius))
                return false;



        if(ax > ay) {
                t = ay - (ax >> 1);
                do {
                        if(t >= 0) {
                                y += sy;
                                t -= ax;
                        }

                        x += sx;
                        t += ay;

                        if(x == goalx && y == goaly) {
                                return true;
                        }
                } while(!blocks_light(y, x));

                return false;
        } else {
                t = ax - (ay >> 1);
                do {
                        if(t >= 0) {
                                x += sx;
                                t -= ay;
                        }

                        y += sy;
                        t += ax;
                        if(x == goalx && y == goaly) {
                                return true;
                        }
                } while(!blocks_light(y, x));

                return false;
        }
}

bool next_to(actor_t *a, actor_t *b)
{
        if(!a)
                return 0;
        if(!b)
                return 0;

        if((a->x == b->x-1 && a->y == b->y) ||
                        (a->x == b->x+1 && a->y == b->y) ||
                        (a->y == b->y-1 && a->x == b->x) ||
                        (a->y == b->y+1 && a->x == b->x) ||
                        (a->x == b->x-1 && a->y == b->y-1) ||
                        (a->x == b->x+1 && a->y == b->y+1) ||
                        (a->x == b->x-1 && a->y == b->y+1) ||
                        (a->x == b->x+1 && a->y == b->y-1))
                return true;
        else
                return false;

        return false;
}

// loaned from the d20 system
int ability_modifier(int ab)
{
        return ((ab / 2) - 5);
}

/*
 * ATTACK!
 */
void attack(actor_t *attacker, actor_t *defender)
{
        int attack, defense, damage;

        defender->attacker = attacker;

        attack  = d(1, 20);
        attack += ability_modifier(attacker->attr.str); // strength_modifier[pstr];
        if(attacker->weapon)
                attack += attacker->weapon->attackmod;

        defense = 10;         // base defense
        defense += ability_modifier(defender->attr.dex);
        // + class/race bonus
        // + equipment  bonus

        if(attacker->weapon) {
                damage = dice(attacker->weapon->dice, attacker->weapon->sides, attacker->weapon->damagemod);
        } else {
                damage = dice(1, 3, 0);
        }

        damage -= defender->ac;       // TODO: Adjust/change 


        //gtprintfc(C_BLACK_MAGENTA, "DEBUG: %s:%d - attack = %d   defense = %d   damage = %d\n", __FILE__, __LINE__, attack, defense, damage);
        if(attack >= defense) {  // it's a hit!
                if(attacker == player) {
                        if(damage == 0)
                                youc(COLOR_INFO, "You hit the %s, but do no damage!", defender->name);
                        else
                                youc(C_BLACK_GREEN, "hit the %s with a %s for %d damage!", defender->name, attacker->weapon ? attacker->weapon->basename : "fistful of nothing", damage);
                } else {
                        if(damage == 0)
                                gtprintfc(COLOR_INFO, "The %s hits you, but does no damage!", attacker->name);
                        else
                                gtprintfc(C_BLACK_RED, "The %s hits you with a %s for %d damage", attacker->name, attacker->weapon ? attacker->weapon->basename : "fistful of nothing", damage);
                }

                defender->hp -= damage;
                if(defender->hp <= 0) {
                        if(defender == player) {
                                you("die!!!");
                        } else {
                                youc(C_BLACK_GREEN, "kill the %s!", defender->name);
                                kill_monster(defender);
                        }
                }
        } else {
                if(attacker == player)
                        youc(C_BLACK_RED, "miss the %s!", defender->name);
                else
                        gtprintfc(C_BLACK_GREEN, "The %s tries to hit you, but fails!", attacker->name);
        }

        if(attacker == player)
                player->ticks -= TICKS_ATTACK;
        else
                attacker->ticks -= TICKS_ATTACK;
}
