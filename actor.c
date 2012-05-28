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
        if(is_invisible(dest)) {
                if(perc(get_intelligence(src)))
                        return in_lineofsight(src, dest->y, dest->x);   //TODO:ADD see invisble stuff.
                else
                        return false;
        } else
                return in_lineofsight(src, dest->y, dest->x);
}

/*
 * This function will check if the actor src can see cell at goaly,goalx
 * Returns true if it can, false if not,
 *
 * The non-libtcod part is adapted from http://roguebasin.roguelikedevelopment.org/index.php/Simple_Line_of_Sight
 */
bool in_lineofsight(actor_t *src, int goaly, int goalx)
{
#ifdef GT_USE_LIBTCOD
        TCOD_map_compute_fov(world->curlevel->map, src->x, src->y, src->viewradius, true, FOV_SHADOW);
        if(TCOD_map_is_in_fov(world->curlevel->map, goalx, goaly))
                return true;
        else
                return false;
#else
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
#endif
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

        if(get_strength(defender) >= 15)
                ret += get_strength(defender) - 13;
        if(get_physique(defender) >= 15)
                ret += get_physique(defender) - 13;
        if(get_intelligence(defender) >= 15)
                ret += get_intelligence(defender) - 13;
        if(get_wisdom(defender) >= 15)
                ret += get_wisdom(defender) - 13;
        if(get_dexterity(defender) >= 15)
                ret += get_dexterity(defender) - 13;
        if(get_charisma(defender) >= 15)
                ret += get_charisma(defender) - 13;

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

void dump_header(char *s)
{
        printf("\n\n\t\t\t * %s *\n", s);
}

void player_die_dump_inventory()
{
        int j;
        obj_t *o;

        dump_header("Inventory");
        
        printf("\t\tGold: %d\n", player->inventory->gold);
        for(j = 0; j < 52; j++) {
                if(player->inventory->object[j]) {
                        o = player->inventory->object[j];
                        identify(o);
                        printf("\t\t%c - %s\n", slot_to_letter(j), a_an(pair(o)));
                }
        }
}

void player_die(actor_t *killer)
{
        if(game->wizardmode) {
                youc(COLOR_RED, "die! Fortunately, you're in wizard mode! 10 HP for you!");
                player->hp += 10;
                return;
        }

        gtprintf(" ");
        youc(COLOR_WHITE, "die...");
        more();
        shutdown_display();

        dump_header(player->name);
        if(!killer->weapon)
                printf("\t\tWas beaten to death by %s,\n", a_an(killer->name));
        else {
                identify(killer->weapon);
                printf("\t\tWas killed by %s, using %s,\n", a_an(killer->name), a_an(killer->weapon->fullname));
        }

        if(game->context == CONTEXT_DUNGEON)
                printf("\t\tat level %d of the dungeon.\n", game->currentlevel);
        if(game->context == CONTEXT_OUTSIDE)
                printf("\t\twhile outside the dungeon.\n");

        player_die_dump_inventory();

        dump_header("Trivia");
        printf("\t\tYou killed a total of %d monsters.\n", player->kills);
        printf("\t\tYou got a total of %d experience points.\n\t\tYou got a final score of %d points.\n", player->xp, calculate_final_score());
        printf("\t\tYou died with %d/%d hitpoints.\n", player->hp, player->maxhp);
        printf("\n\n\n\n\n");


        shutdown_gt();
        exit(0);
}

int attackroll(actor_t *a)
{
        int attack;

        attack = d(1, 20);
        attack += ability_modifier(get_strength(a)); // eller dex?
        if(a->weapon)
                attack += a->weapon->attackmod;

        return attack;
}

int defenseroll(actor_t *defender)
{
        int defense;

        defense = 10;         // base defense
        defense += ability_modifier(get_dexterity(defender));
        defense += defender->ac;
        // + class/race bonus
        // + equipment  bonus


        return defense;
}

int damageroll(actor_t *attacker)
{
        int damage;

        if(attacker->weapon) {
                damage = dice(attacker->weapon->dice, attacker->weapon->sides, (attacker->weapon->damagemod + ability_modifier(get_strength(attacker)))); //TODO: change when we take attributes more into regard depending on weapon (e.g. some require dex, others str, etc.
        } else {
                damage = dice(1, 3, ability_modifier(get_strength(attacker)));
        }

        return damage;
}

/*
 * ATTACK!
 */
void attack(actor_t *attacker, actor_t *defender)
{
        int attack, defense, damage, crit, i;
        bool naturalhit, naturalmiss, criticalmiss, criticalhit;

        naturalmiss = naturalhit = criticalmiss = criticalhit = false;

        // First of all, check if attacker and defender are next to each other, or if one has moved in the meantime
        // Note: I'm not sure if this will actually ever happen...
        if(!next_to(attacker, defender)) {
                if(attacker == player)
                        gtprintf("You try to attack the %s, but the %s moves out of range!", defender->name, defender->name);
                if(defender == player)
                        gtprintf("The %s tries to attack you, but you move out of range!", attacker->name);

                return;
        }

        defender->attacker = attacker;

        /* 
         * First, see if this is a critical hit, or miss.
         */
        attack = d(1,20);
        if(attack == 1) {
                naturalmiss = true;
                criticalmiss = true;
        } else if(attack == 20) {
                naturalhit = true;
                crit = attackroll(attacker);
                defense = defenseroll(defender);
                if(crit >= defense) {    // it's a critical hit!
                        criticalhit = true;
                        damage = damageroll(attacker);
                        damage += damageroll(attacker);
                }
        } else {
                attack  = attackroll(attacker);
                defense = defenseroll(defender);
                damage  = damageroll(attacker);
        }
        
        //gtprintfc(C_BLACK_MAGENTA, "DEBUG: %s:%d - attack = %d   defense = %d   damage = %d\n", __FILE__, __LINE__, attack, defense, damage);
        if(attack >= defense) {  // it's a hit!
                if(attacker == player) {
                        if(damage <= 0) {
                                youc(COLOR_WHITE, "hit the %s, but do no damage!", defender->name);
                        } else {
                                if(criticalhit)
                                        gtprintfc(COLOR_GREEN, "Bullseye! You hit the %s real hard!!", defender->name);
                                else
                                        youc(COLOR_RED, "hit the %s with a %s for %d damage!", defender->name, attacker->weapon ? attacker->weapon->basename : "fistful of nothing", damage);
                        }
                } else {
                        if(damage <= 0) {
                                gtprintfc(COLOR_WHITE, "The %s hits you with a %s, but does no damage!", attacker->name, attacker->weapon ? attacker->weapon->basename : "fistful of nothing");
                        } else {
                                if(criticalhit)
                                        gtprintfc(COLOR_RED, "Ouch! That hurt real bad!");
                                else
                                        gtprintfc(COLOR_RED, "The %s hits you with a %s for %d damage", attacker->name, attacker->weapon ? attacker->weapon->basename : "fistful of nothing", damage);
                        }
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
                if(attacker == player) {
                        if(criticalmiss && naturalmiss) {
                                youc(COLOR_RED, "try to hit the %s, but fail miserably!", defender->name);
                                if(perc(50)) {
                                        i = d(1,8);
                                        switch(i) {
                                                case 1: youc(COLOR_RED, "hit yourself in the left foot!"); break;
                                                case 2: youc(COLOR_RED, "hit yourself in the left arm!"); break;
                                                case 3: youc(COLOR_RED, "hit yourself in the stomach!"); break;
                                                case 4: youc(COLOR_RED, "hit yourself in the head!"); break;
                                                case 5: youc(COLOR_RED, "hit yourself in the right foot!"); break;
                                                case 6: youc(COLOR_RED, "hit yourself in the right arm!"); break;
                                                case 7: youc(COLOR_RED, "hit yourself in the face!"); break;
                                                case 8: youc(COLOR_RED, "hit yourself in the chest!"); break;
                                        }
                                        i = d(1,3);
                                        player->hp -= i;
                                }
                        } else {
                                youc(COLOR_WHITE, "miss the %s!", defender->name);
                        }
                } else {
                        if(criticalmiss && naturalmiss) {
                                gtprintfc(COLOR_GREEN, "The %s tries to hit you, but fails miserably!!", attacker->name);
                                if(perc(75)) {
                                        i = d(1,8);
                                        switch(i) { // TODO: Don't assume that every monster has human bodyparts!
                                                case 1: gtprintfc(COLOR_GREEN, "The %s hits itself in the left foot!", attacker->name); break;
                                                case 2: gtprintfc(COLOR_GREEN, "The %s hits itself in the left arm!", attacker->name); break;
                                                case 3: gtprintfc(COLOR_GREEN, "The %s hits itself in the stomach!", attacker->name); break;
                                                case 4: gtprintfc(COLOR_GREEN, "The %s hits itself in the head!", attacker->name); break;
                                                case 5: gtprintfc(COLOR_GREEN, "The %s hits itself in the right foot!", attacker->name); break;
                                                case 6: gtprintfc(COLOR_GREEN, "The %s hits itself in the right arm!", attacker->name); break;
                                                case 7: gtprintfc(COLOR_GREEN, "The %s hits itself in the face!", attacker->name); break;
                                                case 8: gtprintfc(COLOR_GREEN, "The %s hits itself in the chest!", attacker->name); break;
                                        }
                                        i = d(1,3);
                                        defender->hp -= i;
                                }
                        } else {
                                gtprintfc(COLOR_WHITE, "The %s tries to hit you with a %s, but fails!", attacker->name, attacker->weapon ? attacker->weapon->basename : "fistful of nothing");
                        }
                }
        }

        if(attacker != player)
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

bool get_next_autoexplore_coord(short *x, short *y)
{
        bool found, done;
        found = false;
        done  = false;

        if(check_if_all_explored(world->curlevel)) {
                done = true;
        }

        if(!done) {
                while(!found) {
                        *x = ri(1, world->curlevel->xsize-1);
                        *y = ri(1, world->curlevel->ysize-1);
                        if(ct(*y, *x) == DNG_FLOOR)
                                if(!hasbit(cf(*y, *x), CF_VISITED))
                                        found = true;
                }
        }

        return done;
}

void autoexplore()
{
        int nx, ny, x, y;
        bool done;

        nx = plx; ny = ply;

        done = get_next_autoexplore_coord(&(player->goalx), &(player->goaly));

        if(done) {
                gtprintf("You have explored the entire dungeon.");
        } else {
                setbit(player->flags, PF_AUTOEXPLORING);
                TCOD_path_compute(player->path, plx, ply, player->goalx, player->goaly);
                while(!TCOD_path_is_empty(player->path)) {
                        if(TCOD_path_walk(player->path, &x, &y, true)) {
                                // let's move!
                                
                                if(is_autoexploring) {             // i.e. if not interrupted
                                        if(y > ny) { // moving downward
                                                if(x > nx)
                                                        queuemany(player, ACTION_PLAYER_MOVE_SE, ACTION_HEAL_PLAYER, ENDOFLIST);
                                                if(x < nx)
                                                        queuemany(player, ACTION_PLAYER_MOVE_SW, ACTION_HEAL_PLAYER, ENDOFLIST);
                                                if(x == nx)
                                                        queuemany(player, ACTION_PLAYER_MOVE_DOWN, ACTION_HEAL_PLAYER, ENDOFLIST);
                                        }

                                        if(y < ny) {
                                                if(x > nx)
                                                        queuemany(player, ACTION_PLAYER_MOVE_NE, ACTION_HEAL_PLAYER, ENDOFLIST);
                                                if(x < nx)
                                                        queuemany(player, ACTION_PLAYER_MOVE_NW, ACTION_HEAL_PLAYER, ENDOFLIST);
                                                if(x == nx)
                                                        queuemany(player, ACTION_PLAYER_MOVE_UP, ACTION_HEAL_PLAYER, ENDOFLIST);
                                        }

                                        if(y == ny) {
                                                if(x > nx)
                                                        queuemany(player, ACTION_PLAYER_MOVE_RIGHT, ACTION_HEAL_PLAYER, ENDOFLIST);
                                                if(x < nx)
                                                        queuemany(player, ACTION_PLAYER_MOVE_LEFT, ACTION_HEAL_PLAYER, ENDOFLIST);
                                        }
                                        nx = x; ny = y;
                                        do_turn();
                                }
                        } else {
                                gtprintf("Hm - you seem to be stuck!");
                        }
                }
                clearbit(player->flags, PF_AUTOEXPLORING);
        }
}

int get_strength(actor_t *a)
{
        return a->attr.str + a->attrmod.str;
}

int get_wisdom(actor_t *a)
{
        return a->attr.wis + a->attrmod.wis;
}

int get_dexterity(actor_t *a)
{
        return a->attr.dex + a->attrmod.dex;
}

int get_intelligence(actor_t *a)
{
        return a->attr.intl + a->attrmod.intl;
}

int get_physique(actor_t *a)
{
        return a->attr.phy + a->attrmod.phy;
}

int get_charisma(actor_t *a)
{
        return a->attr.cha + a->attrmod.cha;
}

