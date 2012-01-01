/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Copyright 2011 Rolf Klausen
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

void spawn_monster(int n, monster_t *head)
{
        int i;
        monster_t *tmp;

        tmp = head->next;
        head->next = gtmalloc(sizeof(monster_t));
        memset(head->next, 0, sizeof(monster_t));
        *head->next = get_monsterdef(n);
        head->next->next = tmp;
        head->next->prev = head->next;
        head->next->head = head;
        gtprintf("spawned monster %s\n", head->next->name);
}
