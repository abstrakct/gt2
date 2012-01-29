/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * objects.c - general handling of objects, inventory, etc.
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

unsigned int oid_counter;
char objchars[] = {
        ' ',               // nothing
        ')',               // weapon
        '[',               // armor
        '=',               // ring
        '+',               // card
        '/',               // wand
        '?',               // thing
        '$',               // gold
        '"',               // amulet
        '!',               // potion
};

obj_t get_objdef(int n)
{
        obj_t *tmp;

        tmp = objdefs->head->next;
        while(tmp->id != n) {
                tmp = tmp->next;
        }

        return *tmp;
}

int get_objdef_by_name(char *wanted)
{
        obj_t *o;

        o = objdefs->head->next;
        while(strcmp(o->basename, wanted))
                o = o->next;

        return o->id;
}

void unspawn_object(obj_t *m)
{
        if(m) {
                m->prev->next = m->next;
                if(m->next)
                        m->next->prev = m->prev;
                gtfree(m);
        }
}

obj_t *init_inventory()
{
        obj_t *i;

        i = gtmalloc(sizeof(obj_t));
        i->type = OT_GOLD;

        return i;
}

void pick_up(obj_t *o, void *p)
{
        actor_t *a;

        a = (actor_t *) p;

        if(!a->inventory)
                die("inventory not initialized!");

        o->prev = a->inventory;
        o->next = a->inventory->next;
        a->inventory->next = o;
        o->head = a->inventory;

        if(o->type == OT_WEAPON) {
                a->weapon = o;
                gtprintf("You are now wielding a %s!", o->basename);
        }

        // TODO: tackle cells with multiple items!
        world->curlevel->c[a->y][a->x].inventory->next = NULL;
}

/*
 * Move object *o to inventory *i
 */
bool move_to_inventory(obj_t *o, obj_t *i)
{
        if(o->type == OT_GOLD) {
                i->quantity += o->quantity;
                unspawn_object(o);
                return true;
        } else {
                // TODO: ->prev !!!

                o->head = i->head;
                o->next = i->next;
                if(i->next)
                        i->next->prev = o;
                i->next = o;
                i->sides++;        // use 'sides' in head as counter of how many items are in here.
                //i->type = OT_GOLD; // a strange place to set this, but for now...

                return true;
        }

        return false;
}

/*
 * place a spawned object at (y,x)
 */
bool place_object_at(int y, int x, obj_t *obj, void *p)
{
        level_t *l;

        l = (level_t *) p;
        
        if(l->c[y][x].type == AREA_PLAIN || l->c[y][x].type == DNG_FLOOR) {
                if(!l->c[y][x].inventory)
                        l->c[y][x].inventory = gtmalloc(sizeof(obj_t));

                if(move_to_inventory(obj, l->c[y][x].inventory))
                        return true;
        } else
                return false;

        return false;
}

void spawn_object(int n, obj_t *head)
{
        obj_t *tmp;

        tmp = head->next;
        head->next = gtmalloc(sizeof(obj_t));
        *head->next = get_objdef(n);
        head->next->next = tmp;
        head->next->prev = head;
        head->next->head = head;
        oid_counter++;
        head->next->oid = oid_counter;

        //fprintf(stderr, "spawned obj %s (oid %d)\n", head->next->basename, head->next->oid);
}

void spawn_gold(int n, obj_t *head)
{
        obj_t *tmp;

        tmp = head->next;
        head->next = gtmalloc(sizeof(obj_t));
        head->type = OT_GOLD;
        head->next->next = tmp;
        head->next->prev = head;
        head->next->head = head;
        head->next->quantity = n;
}

/*
 * spawn an object (objdef n) and place it at (y,x) on level
 */
bool spawn_object_at(int y, int x, int n, obj_t *i, void *level)
{
        if(!i)
                i = init_inventory();

        spawn_object(n, i);
        if(!place_object_at(y, x, i->next, (level_t *) level)) {
                //gtprintf("place_obj failed! why?!??");
                unspawn_object(i->next);
                return false;
        }

        return true;
}

bool spawn_gold_at(int y, int x, int n, obj_t *i, void *level)
{
        level_t *l;

        l = (level_t *) level;
        if(l->c[y][x].type == AREA_PLAIN || l->c[y][x].type == DNG_FLOOR) {
                if(!i)
                        i = init_inventory();

                i->quantity += n;
                l->c[y][x].inventory = i;

                return true;
        }

        return false;
}
        
void spawn_golds(int num, int max, void *p)
{
        int i, x, y, n;
        level_t *l;

        i = 0;
        l = (level_t *) p;
        while(i < num) {
                x = ri(1, l->xsize-1);
                y = ri(1, l->ysize-1);
                n = ri(1, max);
                if (spawn_gold_at(y, x, n, l->objects, l)) {
                        i++;
                        //fprintf(stderr, "DEBUG: %s:%d - spawned %d gold at %d,%d\n", __FILE__, __LINE__, n, y, x);
                }
        }
        //fprintf(stderr, "DEBUG: %s:%d - spawn_golds spawned %d golds (should spawn %d)\n", __FILE__, __LINE__, i, num);

}

/*
 * Spawn num objects on level l
 */
void spawn_objects(int num, void *p)
{
        int i, x, y, o;
        level_t *l;

        i = 0;
        l = (level_t *) p;
        while(i < num) {
                x = ri(1, l->xsize-1);
                y = ri(1, l->ysize-1);
                o = ri(1, game->objdefs);
                if (spawn_object_at(y, x, o, l->objects, l)) {
                        i++;
                        //fprintf(stderr, "DEBUG: %s:%d - spawned object %d (objdef %d) at %d,%d\n", __FILE__, __LINE__, i, o, y, x);
                }
        }
        fprintf(stderr, "DEBUG: %s:%d - spawn_objects spawned %d objects (should spawn %d)\n", __FILE__, __LINE__, i, num);
}
