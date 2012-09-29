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
#include "quest.h"
#include "npc.h"
#include "world.h"
#include "datafiles.h"
#include "io.h"
#include "gt.h"
#include "utils.h"

unsigned int mid_counter;
int distancemap[YSIZE][XSIZE];

aifunction aitable[] = {
        simpleai,
        advancedai,
        hostile_ai
};


/**
 * @brief Make a "distance map", used for pathfinding.
 *
 * This function, and get_next_step(), is taken/adapted from:
 *
 * Newsgroups: rec.games.roguelike.development
 * From: "copx" <inva...@dd.com>
 * Date: Mon, 23 Dec 2002 09:57:10 +0100
 * Local: Mon, Dec 23 2002 9:57 am
 * Subject: Re: *simple* pathfinding
 * 
 * @param desty Y coordinate of destination.
 * @param destx X coordinate of destination.
 *
 */
void makedistancemap(int desty, int destx)
{
        int y, x, newdist;
        bool flag;

        //gtprintf("%d - makedistancemap - START!", game->turn);
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
        //gtprintf("%d - makedistancemap - END!", game->turn);
}

/**
 * @brief Very stupid, random "pathfinder". It makes an actor move to a random (and legal/possible) cell.
 *
 * @param m Pointer to the actor/monster which is to do this movement.
 *
 * @return True if actor successfully moved, false if not.
 */
int simpleoutdoorpathfinder(actor_t *m)
{
        int choice;
        int oy, ox;
        //co c;

        oy = m->y;
        ox = m->x;

        if(m->y <= 2)
                return true;
        if(m->x <= 2)
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
                
                m->y += ri(-1, 1);
                m->x += ri(-1, 1);
        }

        //makedistancemap(m->goaly, m->goalx);
        //c = get_next_step(m->y, m->x);

        //m->y += c.y;
        //m->x += c.x;

        if(monster_passable(world->curlevel, m->y, m->x)) {
                world->cmap[oy][ox].monster = NULL;
                world->cmap[m->y][m->x].monster = m;
                return true;
        } else {
                m->y = oy;
                m->x = ox;
                return false;
        }

}

/**
 * @brief A very simple "AI". 
 *
 * @param m Monster/actor who is doing this.
 */
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

        if(monster_passable(world->curlevel, m->y, m->x)) {
                world->cmap[oy][ox].monster = NULL;
                world->cmap[m->y][m->x].monster = m;
        } else {
                m->x = ox; m->y = oy;
        }
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

/**
 * @brief Callback function for libtcod pathfinding.
 *
 * @param xFrom Source X
 * @param yFrom Source Y
 * @param xTo   Dest. X
 * @param yTo   Dest. Y
 * @param user_data Pointer to the level where the pathfinding is taking place. 
 *
 * @return 1.0 if Dest X,Y is passable for a monster, 0.0 if not.
 */
float monster_path_callback_func(int xFrom, int yFrom, int xTo, int yTo, void *user_data)
{
        level_t *l;
        float f;

        l = (level_t*)user_data;
        if(monster_passable(l, yTo, xTo))
                f = 1.0f;
        else
                f = 0.0f;

        return f;
}

/**
 * @brief A simple, but effective AI function which will attack the player or other hostile creatures - or chase them if necessary!
 *
 * @param m The monster/actor which is performing this hostility.
 */
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

        if(next_to(m, player) && !is_invisible(player)) {
                m->attacker = player;
                attack(m, m->attacker);
                return;
        }

        if(actor_in_lineofsight(m, player)) {
                m->goalx = player->x;
                m->goaly = player->y;
        } else {
                m->attacker = NULL;
                do {
                        m->goalx = ri(1, world->curlevel->xsize-1);
                        m->goaly = ri(1, world->curlevel->ysize-1);
                } while(!monster_passable(world->curlevel, m->goaly, m->goalx));
        }

        c = get_next_step(m);
        if(c.x == 0 && c.y == 0) {
                return;
        } else {
                m->y = c.y;
                m->x = c.x;
                world->cmap[oy][ox].monster = NULL;
                world->cmap[m->y][m->x].monster = m;
        }
}

/**
 * @brief Heal a monster, and notify the player.
 *
 * @param m   Pointer to monster to be healed.
 * @param num Number of hitpoints to heal.
 */
void heal_monster(actor_t *m, int num)
{
        increase_hp(m, num);
        gtprintf("The %s looks a bit healthier!", m->name);
}


/**
 * @brief Make a move for a specific monster. Heal the monster if appropriate.
 *
 * @param m Monster to move.
 *
 */
void move_monster(monster_t *m)
{
        int i;

        if(!m) {
                fprintf(stderr, "DEBUG: %s:%d - no such monster!\n", __FILE__, __LINE__);
                return;
        }

        if(hasbit(m->flags, MF_ISDEAD)) {
                //fprintf(stderr, "DEBUG: %s:%d - monster is dead!\n", __FILE__, __LINE__);
                return;
        }

        if(hasbit(m->flags, MF_SLEEPING)) {
                if(actor_in_lineofsight(m, player))
                        clearbit(m->flags, MF_SLEEPING);
        }

        if(!hasbit(m->flags, MF_SLEEPING)) {
                hostile_ai(m);
                if(m->hp < m->maxhp) {
                        i = 17 - m->attr.phy;
                        if(i <= 0)
                                i = 1;
                        if(!game->tick % i) {
                                if(perc(40+m->attr.phy)) {
                                        int j;

                                        j = ability_modifier(m->attr.phy);
                                        if(j < 1)
                                                j = 1;
                                        heal_monster(m, ri(1, j));
                                }
                        }
                }
        }

        schedule_monster(m);
}

/**
 * @brief Schedule all monsters moves in the near future.
 */
void schedule_monsters()
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

                if(m && !hasbit(m->flags, MF_SLEEPING)) {
                        schedule_monster(m);
                }
        }
}

/**
 * @brief See if there are any monsters in players LOS. If so, take note.
 */
void look_for_monsters()
{
        monster_t *m;

        //gtprintf("looking for monsters...");

        m = world->curlevel->monsters;
        if(!m)
                return;


        while(m) {
                m = m->next;
                while(m && hasbit(m->flags, MF_ISDEAD))
                        m = m->next;

                if(m && !hasbit(m->flags, MF_SEENBYPLAYER)) {
                        if(actor_in_lineofsight(player, m)) {
                                setbit(m->flags, MF_SEENBYPLAYER);
                                gtprintfc(COLOR_RED, "%s comes into view!", Upper(a_an(m->name)));
                                schedule_monster(m);
                        }
                }
        }
}

/**
 * @brief Get a monster definition.
 *
 * @param n Which monster definition to get.
 *
 * @return The monster definition.
 */
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
/**
 * @brief Place an already spawned monster at a location. Also, initialize pathfinding for monster.
 *
 * @param y Y coordinate.
 * @param x X coordinate.
 * @param monster Pointer to the spawned monster.
 * @param l Pointer to the level where the monster is to be spawned.
 *
 * @return True if monster can be placed at x,y on this level, false if not.
 */
bool place_monster_at(int y, int x, monster_t *monster, level_t *l)
{
        monster->x = x;
        monster->y = y;
        if(monster_passable(l, y, x) && l->c[monster->y][monster->x].monster == NULL) {
                l->c[monster->y][monster->x].monster = monster;
#ifdef GT_USE_LIBTCOD
                monster->path = TCOD_path_new_using_function(l->xsize, l->ysize, monster_path_callback_func, l, 1.0f);
#endif
                return true;
        } else {
                return false;
        }
}

/**
 * @brief Spawn a monster - e.g. make a copy of a monster definition and make that copy ready to be deployed.
 * HP for the monster will be slightly and randomly adjusted, to provide less predictability/more randomness.
 *
 * @param n Number of the monsterdef to use
 * @param head Pointer to the head of the list of monsters which this monster is to be inserted into.
 * @param maxlevel Maximum level of the monster to be created.
 */
void spawn_monster(int n, monster_t *head, int maxlevel)
{
        monster_t *tmp;
        int hpadj, x;
        char string[50];
        obj_t *o;

        tmp = head->next;
        head->next = gtmalloc(sizeof(monster_t));
        *head->next = get_monsterdef(n);
        hpadj = head->next->level * 2;
        head->next->maxhp += ri((-(hpadj/2)), hpadj);
        head->next->hp = head->next->maxhp;

        head->next->next = tmp;
        head->next->prev = head;
        head->next->head = head;
        setbit(head->next->flags, MF_SLEEPING);
        head->next->viewradius = 12;

        if(head->next->inventory) {
                if(head->next->inventory->object[0]) {
                        strcpy(string, head->next->inventory->object[0]->basename);
                        unspawn_object(head->next->inventory->object[0]);

                        x = get_objdef_by_name(string);
                        o = spawn_object(x, 0);
                        head->next->inventory->object[0] = o;
                        head->next->weapon = o;
                }
                        
                if(head->next->inventory->gold)
                        head->next->inventory->gold = ri(1, 1 + head->next->inventory->gold);
        }

        //gtprintf("spawned monster %s\n", head->next->name);
        
        mid_counter++;
        head->next->mid = mid_counter;
}

/**
 * @brief Kill a monster.
 *
 * @param level Pointer to the level where the killing takes place.
 * @param m Pointer to the unfortunate victim.
 * @param killer Pointer to the entity which did the killing.
 */
void kill_monster(void *level, monster_t *m, actor_t *killer)
{
        int i;
        level_t *l;

        l = (level_t *) level;

        if(l->c[m->y][m->x].monster == m) {
                // we probably should free/remove dead monsters, but something keeps going wrong, cheap cop-out:
                // also, this has it's advantages later (can be used for listing killed monsters).
                setbit(l->c[m->y][m->x].monster->flags, MF_ISDEAD);
                l->c[m->y][m->x].monster = NULL;
                if(killer == player) {
                        player->kills++;
                }

                // and a chance of the monster dropping some or all of its inventory
                if(m->inventory) {
                        if(perc(75)) {
                                for(i=0;i<52;i++) {   // most likely monster won't have 52 items, but you never know...
                                        if(m->inventory->object[i]) {
                                                drop(m, m->inventory->object[i]);
                                        }
                                }

                                if(m->inventory->gold) {
                                        l->c[m->y][m->x].inventory->gold += m->inventory->gold;
                                        m->inventory->gold = 0;
                                }
                                        
                        }
                }
        } else {
                gtprintf("monster's x&y doesn't correspond to cell?");
        }
}

/**
 * @brief "Unspawn" a monster - basically remove it from the game entirely.
 *
 * @param m Pointer to the monster.
 */
void unspawn_monster(monster_t *m)
{
        if(m) {
                m->prev->next = m->next;
                if(m->next)
                        m->next->prev = m->prev;
                gtfree(m);
        }
}

/**
 * @brief Spawn a monster, and (try to) place it at x,y (or y,x..)
 *
 * @param y Y coordinate
 * @param x X coordinate
 * @param n Number of the monsterdef to tuse
 * @param head Head of list to attach this monster to (usually level->monsters)
 * @param level Pointer to the level where monster will be spawned
 * @param maxlevel Maximum level of monster.
 *
 * @return True if successfull, false if it failed.
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
 */
/**
 * @brief Spawn a number of monsters on a certain level.
 * Yeah, level is used for two things, and confusing... i should change the terminology!
 *
 * @param num How many monsters.
 * @param max_level Maximum level of the monsters.
 * @param p The level in which to spawn 'em.
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
                game->num_monsters++;
        }
        //fprintf(stderr, "DEBUG: %s:%d - spawn_monsters spawned %d monsters (should spawn %d)\n", __FILE__, __LINE__, i, num);
}

// vim: fdm=syntax guifont=Terminus\ 8
