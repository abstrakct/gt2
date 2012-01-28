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
        advancedai,
        hostile_ai
};

void simpleoutdoorpathfinder(actor_t *m)
{
        int choice;
        int oy, ox;

        oy = m->y;
        ox = m->x;

        if(!m->goalx || !m->goaly || m->x == m->goalx || m->y == m->goaly) {
                // basically, if we have no goal, or have reached the goal, set a new goal.
                m->goalx = ri(1, world->curlevel->xsize - 1);
                m->goaly = ri(1, world->curlevel->ysize - 1);
        }

        // now, let's try to avoid the stupid diagonal only walk.

        choice = ri(1,100);
        if(choice <= 45) {
                if(m->x > m->goalx)
                        m->x--;
                if(m->x < m->goalx)
                        m->x++;
        } else if(choice > 45 && choice <= 90) {
                if(m->y > m->goaly)
                        m->y--;
                if(m->y < m->goaly)
                        m->y++;
        } else if(choice > 90) {
                // maybe not extremely useful, but adds randomness to the movements,
                // as if the creature's attention was briefly caught by something else..

                switch(choice) {
                        case 91:
                                m->x--;
                                m->y++;
                                break;
                        case 92: 
                                m->y++;
                                break;
                        case 93:
                                m->y++;
                                m->x++;
                                break;
                        case 94:
                                m->x--;
                                break;
                        case 95: 
                                break;
                        case 96:
                                m->x++;
                                break;
                        case 97:
                                m->x--;
                                m->y--;
                                break;
                        case 98:
                                m->y--;
                                break;
                        case 99:
                                m->x++;
                                m->y--;
                                break;
                        case 100:
                                break;
                }
        }

        if(!monster_passable(world->curlevel, m->y, m->x)) {
                m->y = oy;
                m->x = ox;
        }

        world->cmap[oy][ox].monster = NULL;
        world->cmap[m->y][m->x].monster = m;
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
        world->cmap[m->y][m->x].monster = m;
/*        if(m->x < 0)
                m->x = 0;
        if(m->y < 0)
                m->y = 0;*/

}

void advancedai(monster_t *m)
{
        //if(actor_in_lineofsight(m, player))
          //      gtprintf("%d - %s - I can see you!", game->turn, m->name);

        simpleai(m);
}

void hostile_ai(actor_t *m)
{
        int oy, ox;

        oy = m->y;
        ox = m->x;

        if(next_to(m, m->attacker)) {
                attack(m, m->attacker);
                return;
        }

        if(player->x >= (m->x-10) && player->x <= m->x+10 && player->y >= m->y-10 && player->y <= m->y+10) {
                m->goalx = player->x;
                m->goaly = player->y;

                if(m->x > m->goalx) {
                        m->x--;
                        if(m->y == player->y && m->x == player->x)
                                m->x++;
                }

                if(m->x < m->goalx) {
                        m->x++;
                        if(m->y == player->y && m->x == player->x)
                                m->x--;
                }

                if(m->y > m->goaly) {
                        m->y--;
                        if(m->y == player->y && m->x == player->x)
                                m->y++;
                }

                if(m->y < m->goaly) {
                        m->y++;
                        if(m->y == player->y && m->x == player->x)
                                m->y--;
                }

                /*if(next_to(m, player)) {
                        m->attacker = player;
                        player->attacker = m;
                }*/
                if(!monster_passable(world->curlevel, m->y, m->x)) {
                        m->y = oy;
                        m->x = ox;
                }
                world->cmap[oy][ox].monster = NULL;
                world->cmap[m->y][m->x].monster = m;
                m->ticks -= 1000;

        } else {
                m->attacker = NULL;
                simpleoutdoorpathfinder(m);
                m->ticks -= 1000;
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
                while(m && hasbit(m->flags, MF_ISDEAD))
                        m = m->next;

                if(m) {
                        if(m->attacker) {
                                /*if(next_to(m, m->attacker)) {
                                        attack(m, m->attacker);
                                } else {*/
                                        m->ticks += (int) (m->speed*1000);

                                        while(m->ticks >= 1000) {
                                                hostile_ai(m);
                                                draw_world(world->curlevel);
                                                draw_wstat();
                                                update_screen();
                                        }
                        } else {
                                m->ticks += (int) (m->speed*1000);
                                while(m->ticks >= 1000) {
                                        if(m->ai)
                                                m->ai(m);
                                        m->ticks -= 1000;
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
                //gtprintf("place_monster failed! probably tried to spawn at non-passable cell");
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
