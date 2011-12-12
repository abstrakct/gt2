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

#include "monsters.h"
#include "objects.h"
#include "utils.h"
#include "datafiles.h"
#include "world.h"
#include "gt.h"

config_t *cf;

int parse_monsters()
{
        config_setting_t *cfg_monsters;
        int i,j,boolval;
        char sname[100];
        const char *value;

        cfg_monsters = config_lookup(cf, "monsters");
        i = config_setting_length(cfg_monsters);
        printf("Parsing monster file...\n  We have %d monsters", i);

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
        return 0;
}

int parse_armor()
{
        config_setting_t *cfg;
        int i, j;
        char sname[100];
        const char *value;

        cfg = config_lookup(cf, "armor");
        i = config_setting_length(cfg);
        printf("Parsing armor file...\n  We have %d armors", i);
        for(j=0;j<i;j++) {
                obj_t *o;

                o = (obj_t *) gtmalloc(sizeof(obj_t));
                memset(o, 0, sizeof(obj_t));

                sprintf(sname, "armor.[%d].name", j);
                config_lookup_string(cf, sname, &value);
                strcpy(o->basename, value);

                sprintf(sname, "armor.[%d].type", j);
                config_lookup_string(cf, sname, &value);
                if(!strcmp(value, "bodyarmor"))
                        o->type = OT_BODYARMOR;

                sprintf(sname, "armor.[%d].ac", j);
                config_lookup_int(cf, sname, &(o->ac)); 

                o->head = objdefs->head;
                objdefs->next = o;
                o->next = NULL;
                o->prev = objdefs;
                objdefs = o;

                printf(".");
        }

        printf(" OK\n");
        objdefs->head->ddice = i;

        return 0;
}

int parse_objects()
{
        int ret;

        ret = parse_armor();

        return ret;
}

int parse_configfile()
{
        config_setting_t *cfg;
        int i;
        char sname[100];

        cfg = config_lookup(cf, "config");
        i = config_setting_length(cfg);
        printf("Parsing configuration file...");
        if(i > 1) {
                printf("Something is wrong here...?\n");
                return 1;
        }

        sprintf(sname, "config.[0].min_forests");
        config_lookup_int(cf, sname, &game->c.minf);
        sprintf(sname, "config.[0].max_forests");
        config_lookup_int(cf, sname, &game->c.maxf);
        sprintf(sname, "config.[0].min_cities");
        config_lookup_int(cf, sname, &game->c.minc);
        sprintf(sname, "config.[0].max_cities");
        config_lookup_int(cf, sname, &game->c.maxc);
        sprintf(sname, "config.[0].min_villages");
        config_lookup_int(cf, sname, &game->c.minv);
        sprintf(sname, "config.[0].max_villages");
        config_lookup_int(cf, sname, &game->c.maxv);
        sprintf(sname, "config.[0].min_dungeons");
        config_lookup_int(cf, sname, &game->c.mind);
        sprintf(sname, "config.[0].max_dungeons");
        config_lookup_int(cf, sname, &game->c.maxd);

        printf(" OK\n");
        
        return 0;
}
int parse_data_files()
{
        int ret;

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

        /* TODO:
         * This return value stuff makes no sense!!
         */
        ret = parse_configfile();
        ret = parse_monsters();
        ret = parse_objects();

        config_destroy(cf);
        return ret;
}
