/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Various debugging stuff.
 *
 * Copyright 2011 Rolf Klausen
 */
#include <stdio.h>
#include <stdbool.h>

#include "objects.h"
#include "actor.h"
#include "monsters.h"
#include "utils.h"
#include "datafiles.h"
#include "world.h"
#include "display.h"
#include "debug.h"
#include "gt.h"

void dump_monsterdefs()
{
        monster_t *m, *n;
        int i;

        n = monsterdefs->head;
        for(i=0;i<monsterdefs->head->x;i++) {
                m = n->next;
                gtprintf("%s\t%c\nstr\t%d\tphy\t%d\tintl\t%d\twis\t%d\tdex\t%d\tcha\t%d\n", m->name, m->c, m->attr.str, m->attr.phy, m->attr.intl, m->attr.wis, m->attr.dex, m->attr.cha);
                gtprintf("hp\t%d\tthac0\t%d\tlevel\t%d\tspeed\t%.1f\n", m->hp, m->thac0, m->level, m->speed);
                gtprintf("Can use weapon: %s\tCan use armor: %s\tCan have gold: %s\n", m->flags & MF_CANUSEWEAPON ? "Yes" : "No", m->flags & MF_CANUSEARMOR ? "Yes" : "No", m->flags & MF_CANHAVEGOLD ? "Yes" : "No");
                gtprintf("\n");
                n = m;
        }
}

void dump_monsters(monster_t *list)
{
        monster_t *m;

        m = list->next;
        while(m) {
               gtprintf("monsterdump: %s\n", m->name);
               m = m->next;
        }
}

void dump_objects(obj_t *i)
{
        obj_t *o, *n;

        if(!i) {
                gtprintf("No objects here!");
                return;
        }

        n = i;
        while(n) {
                o = n;
                gtprintf("\n");
                gtprintf("OID:      %d\tBasename: %s\tType:     %s", o->oid, o->basename, otypestrings[o->type]);
                if(o->type == OT_GOLD)
                        gtprintf("Amount:   %d\n", o->quantity);
                if(is_armor(o->type))
                        gtprintf("AC:       %d\n", o->ac);
                gtprintf("Attack modifier:%s%d\n", (o->attackmod >= 0 ? " +" : " "), o->attackmod);
                gtprintf("Damage modifier:%s%d\n", (o->damagemod >= 0 ? " +" : " "), o->damagemod);
                gtprintf("Unique:   %s\n", is_unique(o->flags) ? "yes" : "no");
                if(is_weapon(o->type))
                        gtprintf("Damage:   %dd%d\n", o->dice, o->sides);
                
                gtprintf("\n");
                n = o->next;
        }

}

void dump_action_queue()
{
        int i;
        struct actionqueue *tmp;

        tmp = aq;
        i = 0;
        while(tmp) {
                gtprintf("item %d\taction %d\tnum %d\n", i, tmp->action, tmp->num);
                tmp = tmp->next; 
                i++;
        }
}
