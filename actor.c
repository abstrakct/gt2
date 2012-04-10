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
#include "world.h"
#include "datafiles.h"
#include "io.h"
#include "gt.h"
#include "utils.h"

// TODO FIX!
obj_t *objlet[52];    // 52 pointers to objects, a-z & A-Z


//                            x   1   2   3   4   5   6   7   8  9 10 11 12 13 14 15 16 17 18 19 20
//int strength_modifier[20] = { 0, -5, -4, -3, -3, -2, -2, -1, -1, 0, 0, 0, 0, 0, 1, 2, 2, 3, 3, 4, 5 };

int level_table[] = {
            0,          // level "0"
            0,          // level 1
           40,          // level 2
           90,          // level 3
          160,          // etc
          320,
          577,
          666,
         1000,
         2000,
         4000,
         8000,          // FIX Later: are these numbers reasonable?
        16384,
        32768,
        50000, 
};

#define MAX_PLAYER_LEVEL ((sizeof(level_table) / sizeof(int)) - 1)


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

obj_t *get_object_from_letter(char c, inv_t *i)
{
        return i->object[letter_to_slot(c)];
}

int object_to_slot(obj_t *o, inv_t *inv)
{
        int i;

        for(i = 0; i < 52; i++) {
                if(inv->object[i] == o)
                        return i;
        }

        return -1;
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
                } while(!blocks_light(world->curlevel, y, x));

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
                } while(!blocks_light(world->curlevel, y, x));

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

void increase_hp(actor_t *a, int amount)
{
        a->hp += amount;
        if(a->hp > a->maxhp)
                a->hp = a->maxhp;
}

bool player_leveled_up()
{
        if(player->level == MAX_PLAYER_LEVEL)
                return false;
        else if(player->xp < level_table[player->level + 1])
                return false;
        else if(player->xp >= level_table[player->level + 1])        // we'll deal with multiple levels in one go later
                return true;

        return false;
}

void level_up_player()
{
        player->level++;
        player->maxhp += ri(pphy/2, pphy);
        increase_hp(player, d(1, player->level));

        gtprintfc(COLOR_GREEN, "Congratulations! You've reached level %d!", player->level);
        more();

        // TODO: Add other level up effects here!
}

void award_xp(actor_t *defender)
{
        int ret;

        ret = 0;

        if(defender->attr.str >= 15)
                ret += defender->attr.str - 13;
        if(defender->attr.phy >= 15)
                ret += defender->attr.phy - 13;
        if(defender->attr.intl >= 15)
                ret += defender->attr.intl - 13;
        if(defender->attr.wis >= 15)
                ret += defender->attr.wis - 13;
        if(defender->attr.dex >= 15)
                ret += defender->attr.dex - 13;
        if(defender->attr.cha >= 15)
                ret += defender->attr.cha - 13;

        if(defender->maxhp / 10 < 1)
                ret += defender->maxhp * 2;                        // or * 1? or?
        else
                ret += (defender->maxhp * (defender->maxhp / 10));

        player->xp += ret;

        if(player_leveled_up())
                level_up_player();
}

int calculate_final_score()
{
        int score;

        score  = player->xp;
        score += player->kills * player->maxhp;
        score += (player->inventory->gold/2);

        return score;
}

void player_die(actor_t *killer)
{
        if(game->wizardmode) {
                youc(COLOR_RED, "You die! Fortunately, you're in wizard mode! 10 HP for you!");
                player->hp += 10;
                return;
        }

        gtprintf(" ");
        youc(COLOR_WHITE, "die...");
        more();
        shutdown_display();

        printf("\n\n\t\t\t * %s *\n", player->name);
        if(!killer->weapon)
                printf("\t\tWas beaten to death by %s,\n", a_an(killer->name));
        else
                printf("\t\tWas killed by %s, using %s,\n", a_an(killer->name), a_an(killer->weapon->fullname));

        if(game->context == CONTEXT_DUNGEON)
                printf("\t\tat level %d of the dungeon.\n", game->currentlevel);
        if(game->context == CONTEXT_OUTSIDE)
                printf("\t\twhile outside the dungeon.\n");

        printf("\t\tYou killed a total of %d monsters.\n", player->kills);
        printf("\t\tYou got a total of %d experience points.\n\t\tYou got a final score of %d points.\n\n\n\n\n\n", player->xp, calculate_final_score());

        shutdown_gt();
        exit(0);
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
                        if(damage <= 0)
                                youc(COLOR_WHITE, "You hit the %s, but do no damage!", defender->name);
                        else
                                youc(COLOR_RED, "hit the %s with a %s for %d damage!", defender->name, attacker->weapon ? attacker->weapon->basename : "fistful of nothing", damage);
                } else {
                        if(damage <= 0)
                                gtprintfc(COLOR_WHITE, "The %s hits you, but does no damage!", attacker->name);
                        else
                                gtprintfc(COLOR_RED, "The %s hits you with a %s for %d damage", attacker->name, attacker->weapon ? attacker->weapon->basename : "fistful of nothing", damage);
                }

                if(damage > 0)
                        defender->hp -= damage;

                if(defender->hp <= 0) {
                        if(defender == player) {
                                player_die(attacker);
                        } else {
                                youc(COLOR_RED, "kill the %s!", defender->name);
                                kill_monster(world->curlevel, defender, attacker);
                                award_xp(defender);
                        }
                }
        } else {
                if(attacker == player)
                        youc(COLOR_WHITE, "miss the %s!", defender->name);
                else
                        gtprintfc(COLOR_WHITE, "The %s tries to hit you, but fails!", attacker->name);
        }

        if(attacker == player)
                player->ticks -= TICKS_ATTACK;
        else
                attacker->ticks -= TICKS_ATTACK;
}

void move_player_to_stairs_up(int d)
{
        level_t *l;
        int x, y;

        l = &world->dng[d];
        while(1) {
                y = ri(1, l->ysize-1);      // kinda stupid way to do it... TODO: record location of stairs on each level!
                x = ri(1, l->xsize-1);
                if(hasbit(l->c[y][x].flags, CF_HAS_STAIRS_UP)) {
                        player->y = y;
                        player->x = x;
                        return;
                }
        }
}

void move_player_to_stairs_down(int d)
{
        level_t *l;
        int x, y;

        l = &world->dng[d];
        while(1) {
                y = ri(1, l->ysize-1);      // kinda stupid way to do it... TODO: record location of stairs on each level!
                x = ri(1, l->xsize-1);
                if(hasbit(l->c[y][x].flags, CF_HAS_STAIRS_DOWN)) {
                        player->y = y;
                        player->x = x;
                        return;
                }
        }
}
