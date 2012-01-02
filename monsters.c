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

aifunction aitable[] = {
        simpleai,
        advancedai
};

void simpleai(monster_t *m)
{
        int dir;

        dir = ri(1,9);
        world->cmap[m->y][m->x].monster = NULL;
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

        if(m->x < 0)
                m->x = 0;
        if(m->y < 0)
                m->y = 0;

        if(m->x >= XSIZE-1)
                m->x = XSIZE-2;
        if(m->y >= YSIZE-1)
                m->y = YSIZE-2;

        world->cmap[m->y][m->x].monster = m;
}

void advancedai(monster_t *m)
{
        simpleai(m);
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

bool place_monster_at(int x, int y, monster_t *monster, level_t *level)
{
        monster->x = x;
        monster->y = y;
        if(passable(y, x) && level->c[monster->y][monster->x].monster == NULL) {
                level->c[monster->y][monster->x].monster = monster;
                return true;
        } else {
                return false;
        }
}

// TODO: plassere monster på level?
// eller egen funksjon for det?
// *head bor jo i level_t, men.. (eller skal gjøre det..)
//
void spawn_monster(int n, monster_t *head)
{
        monster_t *tmp;

        tmp = head->next;
        head->next = gtmalloc(sizeof(monster_t));
        memset(head->next, 0, sizeof(monster_t));
        *head->next = get_monsterdef(n);
        head->next->next = tmp;
        head->next->prev = head;
        head->next->head = head;
        gtprintf("spawned monster %s\n", head->next->name);
}

void unspawn_monster(monster_t *m)
{
        if(m) {
                m->prev->next = m->next;
                m->next->prev = m->prev;
                free(m);
        }
}

bool spawn_monster_at(int x, int y, int n, monster_t *head, void *level)
{
        spawn_monster(n, head);
        if(!place_monster_at(x, y, head->next, (level_t *) level)) {
                gtprintf("place_monster failed! probably tried to spawn at non-passable cell");
                unspawn_monster(head->next);
                return false;
        }

        return true;
}
