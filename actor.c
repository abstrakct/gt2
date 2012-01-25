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
#include "you.h"
#include "display.h"
#include "gt.h"

#define SGN(a) (((a)<0) ? -1 : 1)

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

void attack(actor_t *attacker, actor_t *victim)
{
        int damage;

        damage = dice(1, 10, 0);
        victim->hp -= damage;

        if(attacker == player)
                you("hit the %s for %d damage!", victim->name, damage);
        else
                gtprintf("The %s hits you for %d damage", attacker->name, damage);

        if(victim->hp <= 0) {
                if(victim == player)
                        you("die!!!");
                else {
                        you("kill the %s!", victim->name);
                        kill_monster(victim);
                }
        }
}
