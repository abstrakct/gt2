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
#include "world.h"
#include "datafiles.h"
#include "io.h"
#include "debug.h"
#include "gt.h"
#include "utils.h"

char *action_name[] = { 
        "Nothing",
        "Player move left",
        "Player move right",
        "Player move up",
        "Player move down",
        "Player move up-left",
        "Player move up-right",
        "Player move down-left",
        "Player move down-right",
        "Pick up",
        "Attack",
        "Move monsters",
        "Enter dungeon",
        "Go down stairs",
        "Go up stairs",
        "Fix view",
        "Wield/wear",
        "Unwield/unwear",
        "Heal player",
        "Make distancemap (obsolete)",
        "Drop",
        "Quaff",
        "Move monster",
        "Player next move",
        "Decrease invisibility counter",
};


void dump_monsterdefs()
{
        monster_t *m, *n;
        int i;

        n = monsterdefs->head;
        for(i=0;i<monsterdefs->head->x;i++) {
                m = n->next;
                gtprintf("%s\t%c\nstr\t%d\tphy\t%d\tintl\t%d\twis\t%d\tdex\t%d\tcha\t%d\n", m->name, m->c, m->attr.str, m->attr.phy, m->attr.intl, m->attr.wis, m->attr.dex, m->attr.cha);
                gtprintf("hp\t%d\t\tlevel\t%d\tspeed\t%.1f\n", m->hp, m->level, m->speed);
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

void dump_objects(inv_t *i)
{
        obj_t *o;
        int j;

        if(!i) {
                gtprintf("No objects here!");
                return;
        }

        for(j = 0; j < 52; j++) {
                o = i->object[j];
                if(o) {
                        gtprintf("\n");
                        gtprintf("OID:      %d\tBasename: %s\tType:     %s", o->oid, o->basename, otypestrings[o->type]);
                        if(o->type == OT_GOLD)
                                gtprintf("Amount:   %d\n", o->quantity);
                        if(is_armor(o))
                                gtprintf("AC:       %d\n", o->ac);
                        gtprintf("Attack modifier:%s%d\n", (o->attackmod >= 0 ? " +" : " "), o->attackmod);
                        gtprintf("Damage modifier:%s%d\n", (o->damagemod >= 0 ? " +" : " "), o->damagemod);
                        gtprintf("Unique:   %s\n", is_unique(o) ? "yes" : "no");
                        if(is_weapon(o))
                                gtprintf("Damage:   %dd%d\n", o->dice, o->sides);

                        gtprintf("\n");
                }
        }

}

void dump_action_queue_old()
{
        int i;
        struct actionqueue *tmp;

        tmp = aq;
        i = 0;
        //gtprintf("---------------------------------------------------------------------------------------------");
        while(tmp) {
                if(i == 0)
                        gtprintf("HEAD -- number of actions in queue: %d", tmp->num);
                else
                        gtprintf("%d:   action %s (%d) - num %d - tick %d", i, action_name[tmp->action], tmp->action, tmp->num, tmp->tick);
                tmp = tmp->next; 
                i++;
        }
        //gtprintf("---------------------------------------------------------------------------------------------");
}

void dump_action_queue()
{
        int i;

        for(i = 0; i < MAXACT; i++) {
                if(act[i].action != ACTION_FREESLOT)
                        gtprintf("%d:   action %s (%d) - tick %d", i, action_name[act[i].action], act[i].action, act[i].tick);
        }
}
