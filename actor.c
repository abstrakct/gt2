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

void assign_slot(obj_t *o)
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


/*
 * This function will check if the actor src can see actor dest
 * Returns true if it can, false if not,
 *
 * Adapted from http://roguebasin.roguelikedevelopment.org/index.php/Simple_Line_of_Sight
 *
 * TODO: Generalize so that this can work for any pair of x,y coordinates!
 */
bool actor_in_lineofsight(actor_t *src, actor_t *dest)
{
        int t, x, y, ax, ay, sx, sy, dx, dy;

        if(src->x == dest->x && src->y == dest->y) // shouldn't actually happen?
                return true;

        dx = dest->x - src->x;
        dy = dest->y - src->y;

        ax = abs(dx) << 1;
        ay = abs(dy) << 1;

        sx = SGN(dx);
        sy = SGN(dy);

        x = src->x;
        y = src->y;

        // This must be changed to a FOV thing!
        if(dest->x > (x + src->viewradius))
                return false;
        if(dest->y > (y + src->viewradius))
                return false;
        if(dest->x < (x - src->viewradius))
                return false;
        if(dest->y < (y - src->viewradius))
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

                        if(x == dest->x && y == dest->y) {
                                return true;
                        }
                } while(!blocks_light(src->y, src->x));

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
                        if(x == dest->x && y == dest->y) {
                                return true;
                        }
                } while(!blocks_light(src->y, src->x));

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

/*
 * ATTACK!
 */
void attack(actor_t *attacker, actor_t *victim)
{
        int damage;
        int hit, tohit;

        victim->attacker = attacker;

        if(attacker->weapon) {
                damage = dice(attacker->weapon->dice, attacker->weapon->sides, attacker->weapon->damagemod);
        } else {
                damage = dice(1, 3, 0);
        }

        // TODO: FIXXX!!!!!!!!!!
        hit = d(1, 20);             // throw 1d20
        tohit = attacker->thac0;
        tohit -= victim->ac;
        if(attacker->weapon)
                tohit += attacker->weapon->attackmod;
        if(tohit < 1)
                tohit = 1;

        
        //gtprintfc(C_BLACK_MAGENTA, "DEBUG: %s:%d - hit = %d    tohit = %d\n", __FILE__, __LINE__, hit, tohit);
        if(hit <= tohit) {
                if(attacker == player)
                        youc(C_BLACK_GREEN, "hit the %s with a %s for %d damage!", victim->name, attacker->weapon ? attacker->weapon->basename : "fistful of nothing", damage);
                else
                        gtprintfc(C_BLACK_RED, "The %s hits you with a %s for %d damage", attacker->name, attacker->weapon ? attacker->weapon->basename : "fistful of nothing", damage);

                victim->hp -= damage;
                if(victim->hp <= 0) {
                        if(victim == player)
                                player->hp += 10;
                        //you("die!!!");
                        else {
                                youc(C_BLACK_GREEN, "kill the %s!", victim->name);
                                kill_monster(victim);
                        }
                }
        } else {
                if(attacker == player)
                        youc(C_BLACK_RED, "miss the %s!", victim->name);
                else
                        gtprintfc(C_BLACK_GREEN, "The %s tries to hit you, but fails!", attacker->name);
        }

        if(attacker == player)
                player->ticks -= TICKS_ATTACK;
        else
                attacker->ticks -= TICKS_ATTACK;
}
