/*
 * Gullible's Travails - 2011 Rewrite!
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

unsigned int mid_counter;

aifunction aitable[] = {
        simpleai,
        advancedai
};

void simpleoutdoorpathfinder(actor_t *creature, actor_t *player)
{
        int choice;

        if(!creature->goalx || !creature->goaly || creature->x == creature->goalx || creature->y == creature->goaly) {
                // basically, if we have no goal, or have reached the goal, set a new goal.
                creature->goalx = ri(1, world->curlevel->xsize - 1);
                creature->goaly = ri(1, world->curlevel->ysize - 1);
        }

        // now, let's try to avoid the stupid diagonal only walk.

        choice = ri(1,100);
        if(choice <= 45) {
                if(creature->x > creature->goalx)
                        creature->x--;
                if(creature->x < creature->goalx)
                        creature->x++;
        } else if(choice > 45 && choice <= 90) {
                if(creature->y > creature->goaly)
                        creature->y--;
                if(creature->y < creature->goaly)
                        creature->y++;
        } else if(choice > 90) {
                // maybe not extremely useful, but adds randomness to the movements,
                // as if the creature's attention was briefly caught by something else..

                switch(choice) {
                        case 91:
                                creature->x--;
                                creature->y++;
                                break;
                        case 92: 
                                creature->y++;
                                break;
                        case 93:
                                creature->y++;
                                creature->x++;
                                break;
                        case 94:
                                creature->x--;
                                break;
                        case 95: 
                                break;
                        case 96:
                                creature->x++;
                                break;
                        case 97:
                                creature->x--;
                                creature->y--;
                                break;
                        case 98:
                                creature->y--;
                                break;
                        case 99:
                                creature->x++;
                                creature->y--;
                                break;
                        case 100:
                                break;
                }
        }
}

void simpleai(monster_t *m)
{
        int dir, ox, oy;

        dir = ri(1,9);
        ox = m->x; oy = m->y;

        switch(dir) {
                case 1:
                        m->x--;
                        m->y++;
                        break;
                case 2: 
                        m->y++;
                        break;
                case 3:
                        m->y++;
                        m->x++;
                        break;
                case 4:
                        m->x--;
                        break;
                case 5: 
                        break;
                case 6:
                        m->x++;
                        break;
                case 7:
                        m->x--;
                        m->y--;
                        break;
                case 8:
                        m->y--;
                        break;
                case 9:
                        m->x++;
                        m->y--;
                        break;
        }

        if(m->x == plx && m->y == ply) {
                m->x = ox; m->y = oy;
                attack(m, player);
        }

        if(!monster_passable(world->curlevel, m->y, m->x)) {
                m->x = ox; m->y = oy;
                return;
        }

        world->cmap[oy][ox].monster = NULL;
        if(m->x < 0)
                m->x = 0;
        if(m->y < 0)
                m->y = 0;

        world->cmap[m->y][m->x].monster = m;
}

void advancedai(monster_t *m)
{
        //if(actor_in_lineofsight(m, player))
          //      gtprintf("%d - %s - I can see you!", game->turn, m->name);

        simpleai(m);
}

void hostile_ai(actor_t *creature, actor_t *player)
{
        if(player->x >= (creature->x-10) && player->x <= creature->x+10 && player->y >= creature->y-10 && player->y <= creature->y+10) {
                creature->goalx = player->x;
                creature->goaly = player->y;

                if(creature->x > creature->goalx) {
                        creature->x--;
                        if(creature->y == player->y && creature->x == player->x)
                                creature->x++;
                }

                if(creature->x < creature->goalx) {
                        creature->x++;
                        if(creature->y == player->y && creature->x == player->x)
                                creature->x--;
                }

                if(creature->y > creature->goaly) {
                        creature->y--;
                        if(creature->y == player->y && creature->x == player->x)
                                creature->y++;
                }

                if(creature->y < creature->goaly) {
                        creature->y++;
                        if(creature->y == player->y && creature->x == player->x)
                                creature->y--;
                }

                /*if(next_to(creature, player)) {
                        creature->attacker = player;
                        player->attacker = creature;
                }*/
        } else {
                creature->attacker = NULL;
                simpleoutdoorpathfinder(creature, player);
        }
}

void move_monsters()
{
        monster_t *m;

        m = world->curlevel->monsters;
        if(!m)
                return;

        while(m) {
                m = m->next;
                if(m)
                        while(hasbit(m->flags, MF_ISDEAD))
                                m = m->next;

                if(m) {
                        if(m->attacker) {
                                if(next_to(m, m->attacker)) {
                                        attack(m, m->attacker);
                                } else {
                                        m->movement += m->speed;
                                        while(m->movement >= 1.0) {
                                                world->curlevel->c[m->y][m->x].monster = NULL;
                                                hostile_ai(m, m->attacker);
                                                world->curlevel->c[m->y][m->x].monster = m;
                                                m->movement -= 1.0;
                                                draw_world(world->curlevel);
                                                draw_wstat();
                                                update_screen();
                                                if(next_to(m, m->attacker)) {
                                                        attack(m, m->attacker);
                                                        return;
                                                }
                                        }
                                }
                        } else {
                                m->movement += m->speed;
                                while(m->movement >= 1.0) {
                                        if(m->ai)
                                                m->ai(m);
                                        m->movement -= 1.0;
                                }
                        }
                }

        }
}

monster_t get_monsterdef(int n)
{
        monster_t *tmp;

        tmp = monsterdefs->head->next;
        while(tmp->id != n) {
                tmp = tmp->next;
        }

        return *tmp;
}

/*
 * place a spawned monster at (y,x)
 */
bool place_monster_at(int y, int x, monster_t *monster, level_t *l)
{
        monster->x = x;
        monster->y = y;
        if(monster_passable(l, y, x) && l->c[monster->y][monster->x].monster == NULL) {
                l->c[monster->y][monster->x].monster = monster;
                return true;
        } else {
                return false;
        }
}

void spawn_monster(int n, monster_t *head)
{
        monster_t *tmp;

        tmp = head->next;
        head->next = gtmalloc(sizeof(monster_t));
        *head->next = get_monsterdef(n);
        head->next->next = tmp;
        head->next->prev = head;
        head->next->head = head;
        //gtprintf("spawned monster %s\n", head->next->name);
        mid_counter++;
        head->next->mid = mid_counter;
}

void kill_monster(monster_t *m)
{
        // sanity check, can't possibly fail, can it?
        if(world->curlevel->c[m->y][m->x].monster == m) {
                // we probably should free/remove dead monsters, but something keeps going wrong, there cheap cop-out:
                setbit(world->curlevel->c[m->y][m->x].monster->flags, MF_ISDEAD);
                world->curlevel->c[m->y][m->x].monster = NULL;

                /*world->curlevel->c[m->y][m->x].monster->prev->next = world->curlevel->c[m->y][m->x].monster->next;
                world->curlevel->c[m->y][m->x].monster->next->prev = world->curlevel->c[m->y][m->x].monster->prev;
                gtfree(m);*/
        } else {
                gtprintf("monster's x&y doesn't correspond to cell?");
        }
}

void unspawn_monster(monster_t *m)
{
        if(m) {
                m->prev->next = m->next;
                if(m->next)
                        m->next->prev = m->prev;
                gtfree(m);
        }
}

/*
 * spawn a monster and place it at (y,x)
 */
bool spawn_monster_at(int y, int x, int n, monster_t *head, void *level)
{
        spawn_monster(n, head);
        if(!place_monster_at(y, x, head->next, (level_t *) level)) {
                gtprintf("place_monster failed! probably tried to spawn at non-passable cell");
                unspawn_monster(head->next);
                return false;
        }

        return true;
}

/*
 * Spawn num monsters of maximum level max_level, on level l
 * (yeah, level is used for two things, and confusing... i should change the terminology!
 */
void spawn_monsters(int num, void *p, int max_level)
{
        int i, x, y, m;
        level_t *l;

        i = 0;
        l = (level_t *) p;
        while(i < num) {
                x = 1; y = 1; m = ri(1, game->monsterdefs);
                while(!spawn_monster_at(y, x, m, l->monsters, l)) { 
                        x = ri(1, l->xsize-1);
                        y = ri(1, l->ysize-1);
                        m = ri(1, game->monsterdefs);
                }

                i++;
        }
        //fprintf(stderr, "DEBUG: %s:%d - spawn_monsters spawned %d monsters (should spawn %d)\n", __FILE__, __LINE__, i, num);
}
