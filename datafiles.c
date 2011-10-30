/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Copyright 2011 Rolf Klausen
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libconfig.h>

#include "utils.h"
#include "monsters.h"
#include "datafiles.h"
#include "gt.h"

int parse_monsters()
{
        config_t *cf;
        config_setting_t *cfg_monsters;
        //config_setting_t *setting;
        int i,j,boolval;
        char sname[100];
        const char *value;

        cf = (config_t *) gtmalloc(sizeof(config_t));
        config_init(cf);

        if (!config_read_file(cf, MAIN_DATA_FILE)) {
                fprintf(stderr, "%s:%d - %s\n",
                                config_error_file(cf),
                                config_error_line(cf),
                                config_error_text(cf));
                config_destroy(cf);
                return(1);
        }

        printf("Reading %s\n", MAIN_DATA_FILE);
        cfg_monsters = config_lookup(cf, "monsters");
        i = config_setting_length(cfg_monsters);
        printf("Parsing monster file...\nWe have %d monsters", i);

        /* 
         * main monster parsing loop 
         * goes through each possible setting, looks it up in the cfg file
         * and adds it to a monster structure which is then placed in the
         * linked list monsterdefs.
         *
         */
        
        for(j=0;j<i;j++) {
                monster_t *m;

                m = (monster_t *) gtmalloc(sizeof(monster_t));
                memset(m, 0, sizeof(monster_t));

                sprintf(sname, "monsters.[%d].name", j);
                config_lookup_string(cf, sname, &value);
                strcpy(m->name, value);

                sprintf(sname, "monsters.[%d].str", j);
                config_lookup_int(cf, sname, &(m->attr.str));
                sprintf(sname, "monsters.[%d].phys", j);
                config_lookup_int(cf, sname, &(m->attr.phys));
                sprintf(sname, "monsters.[%d].intl", j);
                config_lookup_int(cf, sname, &(m->attr.intl));
                sprintf(sname, "monsters.[%d].know", j);
                config_lookup_int(cf, sname, &(m->attr.know));
                sprintf(sname, "monsters.[%d].dex", j);
                config_lookup_int(cf, sname, &(m->attr.dex));
                sprintf(sname, "monsters.[%d].cha", j);
                config_lookup_int(cf, sname, &(m->attr.cha));

                sprintf(sname, "monsters.[%d].appearance", j);
                config_lookup_string(cf, sname, &value);
                m->c = (char) *value;

                sprintf(sname, "monsters.[%d].level", j);
                config_lookup_int(cf, sname, &(m->level));

                sprintf(sname, "monsters.[%d].hp", j);
                config_lookup_int(cf, sname, &(m->hp));
                m->maxhp = m->hp;

                sprintf(sname, "monsters.[%d].speed", j);
                config_lookup_float(cf, sname, &(m->speed));

                sprintf(sname, "monsters.[%d].thac0", j);
                config_lookup_int(cf, sname, &(m->thac0));

                sprintf(sname, "monsters.[%d].havegold", j);
                if(config_lookup_bool(cf, sname, &boolval))
                        if(boolval)
                                m->flags |= MF_CANHAVEGOLD;

                sprintf(sname, "monsters.[%d].useweapon", j);
                if(config_lookup_bool(cf, sname, &boolval))
                        if(boolval)
                                m->flags |= MF_CANUSEWEAPON;

                sprintf(sname, "monsters.[%d].usearmor", j);
                if(config_lookup_bool(cf, sname, &boolval))
                        if(boolval)
                                m->flags |= MF_CANUSEARMOR;

                sprintf(sname, "monsters.[%d].usesimplesword", j);
                if(config_lookup_bool(cf, sname, &boolval))
                        if(boolval)
                                m->flags |= MF_CANUSESIMPLESWORD;

                /*
                 * the following was written in one go, it's beautiful and seems totally bugfree!!
                 */
                m->head = monsterdefs->head;
                monsterdefs->next = m;
                m->next = NULL;
                m->prev = monsterdefs;
                monsterdefs = m;

                printf("."); // simple "progress bar"
        }
        
        printf(" OK\n");

        monsterdefs->head->x = i; // store number of monsters in x of head.

        config_destroy(cf);
        return 0;
}

int parse_armor()
{
        return 0;
}

int parse_objects()
{
        int ret;

        ret = parse_armor();

        return ret;
}

int parse_data_files()
{
        int ret;

        ret = parse_monsters();
        ret = parse_objects();

        return ret;
}
