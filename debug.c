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
#include "you.h"
#include "display.h"
#include "debug.h"
#include "gt.h"

void dump_monsters()
{
        monster_t *m, *n;
        int i;

        n = monsterdefs->head;
        for(i=0;i<monsterdefs->head->x;i++) {
                m = n->next;
                printf("%s\t%c\nstr\t%d\tphys\t%d\tintl\t%d\tknow\t%d\tdex\t%d\tcha\t%d\n", m->name, m->c, m->attr.str, m->attr.phys, m->attr.intl, m->attr.know, m->attr.dex, m->attr.cha);
                printf("hp\t%d\tthac0\t%d\tlevel\t%d\tspeed\t%.1f\n", m->hp, m->thac0, m->level, m->speed);
                printf("Can use weapon: %s\tCan use armor: %s\tCan have gold: %s\n", m->flags & MF_CANUSEWEAPON ? "Yes" : "No", m->flags & MF_CANUSEARMOR ? "Yes" : "No", m->flags & MF_CANHAVEGOLD ? "Yes" : "No");
                printf("\n");
                n = m;
        }
}

void dump_objects()
{
        obj_t *o, *n;
        int i;

        n = objdefs->head;
        for(i=0; i<objdefs->head->ddice; i++) {
                o = n->next;
                printf("Basename: %s\n", o->basename);
                printf("Type:     %s\n", otypestrings[o->type]);
                if(isarmor(o))
                        printf("AC:       %d\n", o->ac);
                printf("\n");
                n = o;
        }
}

/* mainly a debugging function */
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
