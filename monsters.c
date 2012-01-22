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

        if(!monster_passable(world->curlevel, m->y, m->x)) {
                m->x = ox; m->y = oy;
                return;
        }

        world->cmap[oy][ox].monster = NULL;
        if(m->x < 0)
                m->x = 0;
        if(m->y < 0)
                m->y = 0;

        /*if(m->x >= XSIZE-1)
                m->x = XSIZE-2;
        if(m->y >= YSIZE-1)
                m->y = YSIZE-2;*/

        world->cmap[m->y][m->x].monster = m;
}

void advancedai(monster_t *m)
{
        if(actor_in_lineofsight(m, player))
                gtprintf("%d - %s - I can see you!", game->turn, m->name);

        simpleai(m);
}

void move_monsters()
{
        monster_t *m;

        m = world->curlevel->monsters;
        if(m)
                m = m->next;
        else
                return;

        while(m) {
                if(m->ai)
                        m->ai(m);
                m = m->next;
        }
}

monster_t get_monsterdef(int n)
{
        int i;
        monster_t *tmp;

        tmp = monsterdefs->head;
        for(i=0; i<n; i++) {
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
}
