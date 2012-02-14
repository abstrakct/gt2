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
#include "display.h"
#include "gt.h"

unsigned int mid_counter;
int distancemap[800][800];

aifunction aitable[] = {
        simpleai,
        advancedai,
        hostile_ai
};


/*
 * This function, and get_next_step(), is taken/adapted from:
 *
 * Newsgroups: rec.games.roguelike.development
 * From: "copx" <inva...@dd.com>
 * Date: Mon, 23 Dec 2002 09:57:10 +0100
 * Local: Mon, Dec 23 2002 9:57 am
 * Subject: Re: *simple* pathfinding
 * 
 */
void makedistancemap(int desty, int destx)
{
        int y, x, newdist;
        bool flag;

        for(y = 0; y < world->curlevel->ysize; y++) {
                for(x = 0; x < world->curlevel->xsize; x++) {
                        distancemap[y][x] = 99999;
                }
        }

        distancemap[desty][destx] = 0;
        flag = true;
        while(flag) {
                flag = false;
                for(y = 1; y < world->curlevel->ysize; y++) {
                        for(x = 1; x < world->curlevel->xsize; x++) {
                                if(monster_passable(world->curlevel, y, x)) {
                                        newdist = min(min(2+distancemap[y][x-1], 2+distancemap[y-1][x]), min(3+distancemap[y-1][x-1], 3+distancemap[y-1][x+1]));
                                        if(newdist < distancemap[y][x]) {
                                                distancemap[y][x] = newdist;
                                                flag = true;
                                        }
                                }
                        }
                }

                for(y = world->curlevel->ysize - 3; y >= 1; --y) {
                        for(x = world->curlevel->xsize - 3; x >= 1; --x) {
                                if(monster_passable(world->curlevel, y, x)) {
                                        newdist = min(min(2+distancemap[y][x+1], 2+distancemap[y+1][x]), min(3+distancemap[y+1][x+1], 3+distancemap[y+1][x-1]));
                                        if(newdist < distancemap[y][x]) {
                                                distancemap[y][x] = newdist;
                                                flag = true;
                                        }
                                }
                        }
                }
        }
}

co get_next_step(int y, int x)
{
        co c;
        int dx, dy, dist, newdist, newdx, newdy;

        dx = 0; 
        dy = 0;
        dist = 99999;
        for(newdy = -1; newdy <= 1; newdy++) {
                for(newdx = -1; newdx <= 1; newdx++) {
                        newdist = distancemap[y + newdy][x + newdx];
                        if(newdist < dist) {
                                dist = newdist;
                                dx = newdx;
                                dy = newdy;
                        }
                }
        }

        c.x = dx;
        c.y = dy;
        return c;
}

int simpleoutdoorpathfinder(actor_t *m)
{
        //int choice;
        int oy, ox;
        co c;

        oy = m->y;
        ox = m->x;

        if(m->y <= 1)
                return true;
        if(m->x <= 1)
                return true;

        if(!m->goalx || !m->goaly || m->x == m->goalx || m->y == m->goaly) {
                // basically, if we have no goal, or have reached the goal, set a new goal.
                m->goalx = ri(1, world->curlevel->xsize - 1);
                m->goaly = ri(1, world->curlevel->ysize - 1);
                while(!monster_passable(world->curlevel, m->goaly, m->goalx)) {
                        m->goalx = ri(1, world->curlevel->xsize - 1);
                        m->goaly = ri(1, world->curlevel->ysize - 1);
                }
        }


        makedistancemap(m->goaly, m->goalx);
        c = get_next_step(m->y, m->x);
        m->y += c.y;
        m->x += c.x;

        if(!monster_passable(world->curlevel, m->y, m->x)) {
                m->y = oy;
                m->x = ox;
                return false;
        }

        world->cmap[oy][ox].monster = NULL;
        world->cmap[m->y][m->x].monster = m;
        return true;
}

void simpleai(monster_t *m)
{
        int dir, ox, oy;

        //gtprintf("hello it's simpleai!");
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
        simpleai(m);
}

void newpathfinder_chaseplayer(actor_t *m)
{
        int dx, dy;

        dx = player->x - m->x;
        dy = player->y - m->y;

        dx = max(min(1, dx), -1);
        dy = max(min(1, dy), -1);

        m->y += dy;
        m->x += dx;
}


bool newpathfinder(actor_t *m)
{
        int oy, ox, dx, dy, dist, newdist, newdx, newdy;

        if(!m->goalx || !m->goaly || m->x == m->goalx || m->y == m->goaly) {
                m->goalx = ri(1, world->curlevel->xsize - 1);
                m->goaly = ri(1, world->curlevel->ysize - 1);
                while(!monster_passable(world->curlevel, m->goaly, m->goalx)) {
                        m->goalx = ri(1, world->curlevel->xsize - 1);
                        m->goaly = ri(1, world->curlevel->ysize - 1);
                }
        }

        oy = m->y;
        ox = m->x;
        makedistancemap(m->goaly, m->goalx);

        dx = 0; 
        dy = 0;
        dist = 99999;

        for(newdy = -1; newdy <= 1; newdy++) {
                for(newdx = -1; newdx <= 1; newdx++) {
                        newdist = distancemap[m->y + newdy][m->x + newdx];
                        if(newdist < dist) {
                                dist = newdist;
                                dx = newdx;
                                dy = newdy;
                        }
                }
        }

        m->y += dy;
        m->x += dx;

        /*
        if(!monster_passable(world->curlevel, m->y, m->x)) {
                m->y = oy;
                m->x = ox;
                return false;
        }
        */

        world->cmap[oy][ox].monster = NULL;
        world->cmap[m->y][m->x].monster = m;
        return true;
}


void hostile_ai(actor_t *m)
{
        int oy, ox;
        co c;

        oy = m->y;
        ox = m->x;

        if(m->attacker && next_to(m, m->attacker)) {
                attack(m, m->attacker);
                return;
        }

        if(next_to(m, player)) {
                m->attacker = player;
                attack(m, m->attacker);
                return;
        }

        if(actor_in_lineofsight(m, player)) {
                c = get_next_step(m->y, m->x);

                m->y += c.y;
                m->x += c.x;

                world->cmap[oy][ox].monster = NULL;
                world->cmap[m->y][m->x].monster = m;
        } else {
                m->attacker = NULL;
                while(!simpleoutdoorpathfinder(m));
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

                if(m && hasbit(m->flags, MF_SLEEPING)) {
                        if(actor_in_lineofsight(m, player))
                                clearbit(m->flags, MF_SLEEPING);
                }


                // TODO: SIMPLIFY!
                //

                if(m && !hasbit(m->flags, MF_SLEEPING)) {
                        if(m->attacker) {
                                m->ticks += (int) (m->speed*1000);

                                while(m->ticks >= 1000) {
                                        hostile_ai(m);
                                        m->ticks -= 1000;
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

void spawn_monster(int n, monster_t *head, int maxlevel)
{
        monster_t *tmp;

        tmp = head->next;
        head->next = gtmalloc(sizeof(monster_t));
        *head->next = get_monsterdef(n);

        head->next->next = tmp;
        head->next->prev = head;
        head->next->head = head;
        setbit(head->next->flags, MF_SLEEPING);
        //gtprintf("spawned monster %s\n", head->next->name);
        
        mid_counter++;
        head->next->mid = mid_counter;
        game->num_monsters++;
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
bool spawn_monster_at(int y, int x, int n, monster_t *head, void *level, int maxlevel)
{
        spawn_monster(n, head, maxlevel);
        if(head->next->level > maxlevel) {
                unspawn_monster(head->next);
                return false;
        }

        if(!place_monster_at(y, x, head->next, (level_t *) level)) {
                unspawn_monster(head->next);
                return false;
        }

        return true;
}

/*
 * Spawn num monsters of maximum level max_level, on level l
 * (yeah, level is used for two things, and confusing... i should change the terminology!
 */
void spawn_monsters(int num, int max_level, void *p)
{
        int i, x, y, m;
        level_t *l;

        i = 0;
        l = (level_t *) p;
        while(i < num) {
                x = 1; y = 1; m = ri(1, game->monsterdefs);
                while(!spawn_monster_at(y, x, m, l->monsters, l, max_level)) { 
                        x = ri(1, l->xsize-1);
                        y = ri(1, l->ysize-1);
                        m = ri(1, game->monsterdefs);
                }

                i++;
        }
        //fprintf(stderr, "DEBUG: %s:%d - spawn_monsters spawned %d monsters (should spawn %d)\n", __FILE__, __LINE__, i, num);
}
