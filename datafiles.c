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
#include <stdbool.h>

#include "objects.h"
#include "o_effects.h"
#include "actor.h"
#include "monsters.h"
#include "utils.h"
#include "world.h"
#include "datafiles.h"
#include "gt.h"

config_t *cf;
int objid;             // to keep track of all parsed objects, to give each a unique ID
roomdef_t r;

int parse_roomdef_file(char *filename)
{
        FILE *f;
        int y, x, i, j;
        char c;

        f = fopen(filename, "r");
        if(!f)
                return 1;
        fscanf(f, "%d,%d\n", &y, &x);
        r.y = y;
        r.x = x;
        r.c = (cell_t **) gtmalloc2d(y, x, sizeof(cell_t));

        for(i = 0; i < y; i++) {
                for(j = 0; j < x; j++) {
                        fscanf(f, "%c", &c);
                        r.c[i][j].type = c;
                        printf("%c", r.c[i][j].type);
                        switch(c) {
                                case '#': r.c[i][j].type = DNG_WALL; break;
                                case '.': r.c[i][j].type = DNG_FLOOR; break;
                                case '+': r.c[i][j].type = DNG_FLOOR; 
                                          setbit(r.c[i][j].flags, CF_HAS_DOOR_CLOSED); break;
                                case 't': break;
                                case 'm': break;
                                default: break;
                        }
                }
                fscanf(f, "\n");
                printf("\n");
        }

        return 0;
}

int parse_roomdef_files()
{
        return parse_roomdef_file("data/room/despair.1.room");
}

int parse_monsters()
{
        config_setting_t *cfg_monsters;
        int i, j, boolval, tmp, id;
        char sname[100];
        const char *value;

        cfg_monsters = config_lookup(cf, "monsters");
        i = config_setting_length(cfg_monsters);
        game->monsterdefs = i;
        printf("Parsing monster file... We have %d monsters", i);

        /* 
         * main monster parsing loop 
         * goes through each possible setting, looks it up in the cfg file
         * and adds it to a monster structure which is then placed in the
         * linked list monsterdefs.
         */
        
        for(j=0;j<i;j++) {
                monster_t *m;

                m = (monster_t *) gtmalloc(sizeof(monster_t));
                id = j+1;

                sprintf(sname, "monsters.[%d].name", j);
                config_lookup_string(cf, sname, &value);
                strcpy(m->name, value);

                sprintf(sname, "monsters.[%d].str", j);
                config_lookup_int(cf, sname, &(m->attr.str));
                sprintf(sname, "monsters.[%d].phy", j);
                config_lookup_int(cf, sname, &(m->attr.phy));
                sprintf(sname, "monsters.[%d].intl", j);
                config_lookup_int(cf, sname, &(m->attr.intl));
                sprintf(sname, "monsters.[%d].wis", j);
                config_lookup_int(cf, sname, &(m->attr.wis));
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
                                setbit(m->flags, MF_CANHAVEGOLD);

                sprintf(sname, "monsters.[%d].useweapon", j);
                if(config_lookup_bool(cf, sname, &boolval))
                        if(boolval)
                                setbit(m->flags, MF_CANUSEWEAPON);

                sprintf(sname, "monsters.[%d].usearmor", j);
                if(config_lookup_bool(cf, sname, &boolval))
                        if(boolval)
                                setbit(m->flags, MF_CANUSEARMOR);

                sprintf(sname, "monsters.[%d].usesimplesword", j);
                if(config_lookup_bool(cf, sname, &boolval))
                        if(boolval)
                                setbit(m->flags, MF_CANUSESIMPLESWORD);

                sprintf(sname, "monsters.[%d].aitype", j);
                config_lookup_int(cf, sname, &tmp);
                m->ai = aitable[tmp];
                m->mid = tmp;   // for monsterdefs, mid holds aitableindex
                m->id = id;

                m->viewradius = 12; // temporary solution?!

                // Let's give the monster a weapon!
                if(hasbit(m->flags, MF_CANUSESIMPLESWORD)) {
                        int x;
                        obj_t *o;

                        x = get_objdef_by_name("short sword");

                        m->inventory = init_inventory();
                        o = spawn_object(x, 0);
                        m->inventory->object[0] = o;
                        m->weapon = o;
                }

                if(hasbit(m->flags, MF_CANUSEWEAPON) && !hasbit(m->flags, MF_CANUSESIMPLESWORD)) {
                        int x;
                        obj_t *o;

                        x = get_objdef_by_name("dagger");

                        m->inventory = init_inventory();
                        o = spawn_object(x, 0);
                        m->inventory->object[0] = o;
                        m->weapon = o;
                }

                if(hasbit(m->flags, MF_CANHAVEGOLD)) {
                        if(!m->inventory) {
                                m->inventory = init_inventory();
                                m->inventory->gold += ri(2, 20);
                        }
                }

                
//if(m->weapon)
//fprintf(stderr, "DEBUG: %s:%d - %s has the weapon called a %s\n", __FILE__, __LINE__, m->name, m->weapon->basename);
                        

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
        printf("Parsing armor file... We have %d armors", i);
        for(j=0;j<i;j++) {
                obj_t *o;
                int x;

                o = (obj_t *) gtmalloc(sizeof(obj_t));

                sprintf(sname, "armor.[%d].name", j);
                config_lookup_string(cf, sname, &value);
                strcpy(o->basename, value);

                sprintf(sname, "armor.[%d].type", j);
                config_lookup_string(cf, sname, &value);
                if(!strcmp(value, "bodyarmor"))
                        o->flags |= OF_BODYARMOR;
                if(!strcmp(value, "headarmor"))
                        o->flags |= OF_HEADGEAR;
                if(!strcmp(value, "shield"))
                        o->flags |= OF_SHIELD;
                if(!strcmp(value, "gloves"))
                        o->flags |= OF_GLOVES;
                if(!strcmp(value, "footarmor"))
                        o->flags |= OF_FOOTWEAR;

                o->type   = OT_ARMOR;
                sprintf(sname, "armor.[%d].ac", j);
                config_lookup_int(cf, sname, &x);
                o->ac = x;

                o->id = objid; objid++;

                o->head = objdefs->head;
                objdefs->next = o;
                o->next = NULL;
                objdefs = o;

                printf(".");
        }

        printf(" OK\n");
        objdefs->head->dice = i;
        game->objdefs = i;

        return 0;
}

int parse_weapons()
{
        config_setting_t *cfg;
        int i, j;
        char sname[100];
        const char *value;

        cfg = config_lookup(cf, "weapon");
        i = config_setting_length(cfg);
        printf("Parsing weapon file... We have %d weapons", i);
        for(j=0;j<i;j++) {
                obj_t *o;
                int x;

                o = (obj_t *) gtmalloc(sizeof(obj_t));

                sprintf(sname, "weapon.[%d].name", j);
                config_lookup_string(cf, sname, &value);
                strcpy(o->basename, value);

                sprintf(sname, "weapon.[%d].type", j);
                config_lookup_string(cf, sname, &value);
                if(!strcmp(value, "sword"))
                        o->flags |= OF_SWORD;
                if(!strcmp(value, "axe"))
                        o->flags |= OF_AXE;
                if(!strcmp(value, "knife"))
                        o->flags |= OF_KNIFE;
                if(!strcmp(value, "stick"))
                        o->flags |= OF_STICK;
                if(!strcmp(value, "mace"))
                        o->flags |= OF_MACE;
                if(!strcmp(value, "hammer"))
                        o->flags |= OF_HAMMER;

                o->type   = OT_WEAPON;

                sprintf(sname, "weapon.[%d].dice", j);
                config_lookup_int(cf, sname, &x);
                o->dice = x;
                sprintf(sname, "weapon.[%d].sides", j);
                config_lookup_int(cf, sname, &x);
                o->sides = x;

                sprintf(sname, "weapon.[%d].unique", j);
                config_lookup_bool(cf, sname, &x);
                if(x)
                        o->flags |= OF_UNIQUE;

                x = 0;
                sprintf(sname, "weapon.[%d].mod", j);
                config_lookup_int(cf, sname, &x);
                o->attackmod = o->damagemod = x;

                o->id = objid; objid++;

                o->head = objdefs->head;
                objdefs->next = o;
                o->next = NULL;
                objdefs = o;

                game->objdefs++;
                printf(".");
        }

        printf(" OK\n");

        return 0;
}

int parse_amulet()
{
        config_setting_t *cfg;
        int i, j, material;
        char sname[100];
        const char *value;

        cfg = config_lookup(cf, "amulet");
        i = config_setting_length(cfg);
        printf("Parsing jewelry file... We have %d amulets", i);
        material = 1;
        for(j=0;j<i;j++) {
                obj_t *o;
                int x;

                o = (obj_t *) gtmalloc(sizeof(obj_t));

                sprintf(sname, "amulet.[%d].name", j);
                config_lookup_string(cf, sname, &value);
                strcpy(o->basename, value);
                
                sprintf(sname, "amulet.[%d].brand", j);
                config_lookup_string(cf, sname, &value);
                if(!strcmp(value, "protection")) {                     // This means this amulet is some sort of protection
                        sprintf(sname, "amulet.[%d].type", j);
                        config_lookup_string(cf, sname, &value);

                        if(!strcmp(value, "life")) 
                                add_effect(o, OE_PROTECTION_LIFE);
                        if(!strcmp(value, "fire"))
                                add_effect(o, OE_PROTECTION_FIRE);
                }

                sprintf(sname, "amulet.[%d].unique", j);
                config_lookup_bool(cf, sname, &x);
                if(x)
                        setbit(o->flags, OF_UNIQUE);
                
                sprintf(sname, "amulet.[%d].obvious", j);
                config_lookup_bool(cf, sname, &x);
                if(x)
                        setbit(o->flags, OF_OBVIOUS);

                x = 0;
                /*sprintf(sname, "amulet.[%d].mod", j);
                config_lookup_int(cf, sname, &x);
                o->attackmod = o->damagemod = x;*/

                o->type = OT_AMULET;
                o->id = objid; objid++;

                o->material = mats_amulets[material];
                material++;
                if(material > MATERIALS)
                        die("whoa! we ran out of material!");

                o->head = objdefs->head;
                objdefs->next = o;
                o->next = NULL;
                objdefs = o;

                game->objdefs++;
                printf(".");
        }

        printf(" OK\n");

        return 0;
}

int parse_bracelet()
{
        config_setting_t *cfg;
        int i, j, material;
        char sname[100];
        const char *value;

        cfg = config_lookup(cf, "bracelet");
        i = config_setting_length(cfg);
        printf("Parsing jewelry file... We have %d bracelets", i);
        material = 1;
        for(j = 0; j < i; j++) {
                obj_t *o;
                int x;

                o = (obj_t *) gtmalloc(sizeof(obj_t));

                sprintf(sname, "bracelet.[%d].name", j);
                config_lookup_string(cf, sname, &value);
                strcpy(o->basename, value);
                
                sprintf(sname, "bracelet.[%d].brand", j);
                config_lookup_string(cf, sname, &value);
                if(!strcmp(value, "stat")) {                     // This means this bracelet modifies a stat
                        sprintf(sname, "bracelet.[%d].stat", j);
                        config_lookup_string(cf, sname, &value);

                        if(!strcmp(value, "strength")) 
                                add_effect(o, OE_STRENGTH);
                        if(!strcmp(value, "physique"))
                                add_effect(o, OE_PHYSIQUE);
                        if(!strcmp(value, "intelligence"))
                                add_effect(o, OE_INTELLIGENCE);
                        if(!strcmp(value, "wisdom"))
                                add_effect(o, OE_WISDOM);
                        if(!strcmp(value, "dexterity"))
                                add_effect(o, OE_DEXTERITY);
                        if(!strcmp(value, "charisma"))
                                add_effect(o, OE_CHARISMA);
                }

                sprintf(sname, "bracelet.[%d].unique", j);
                config_lookup_bool(cf, sname, &x);
                if(x)
                        setbit(o->flags, OF_UNIQUE);

                x = 0;
                sprintf(sname, "bracelet.[%d].obvious", j);
                config_lookup_bool(cf, sname, &x);
                if(x)
                        setbit(o->flags, OF_OBVIOUS);

                x = 0;
                sprintf(sname, "bracelet.[%d].mod", j);
                config_lookup_int(cf, sname, &x);
                o->attackmod = o->damagemod = x;

                o->type = OT_BRACELET;
                o->id = objid; objid++;
                clearbit(o->flags, OF_IDENTIFIED);

                o->material = mats_bracelets[material];
                material++;
                if(material > MATERIALS)
                        die("whoa! we ran out of material!");

                o->head = objdefs->head;
                objdefs->next = o;
                o->next = NULL;
                objdefs = o;

                game->objdefs++;
                printf(".");
        }

        printf(" OK\n");

        return 0;
}

int parse_jewelry()
{
        int ret;

        ret = parse_bracelet();
        ret = parse_amulet();

        return ret;
}

int parse_objects()
{
        int ret;

        ret = parse_armor();
        ret = parse_weapons();
        ret = parse_jewelry();

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
        config_lookup_int(cf, sname, &gtconfig.minf);
        sprintf(sname, "config.[0].max_forests");
        config_lookup_int(cf, sname, &gtconfig.maxf);
        sprintf(sname, "config.[0].min_cities");
        config_lookup_int(cf, sname, &gtconfig.minc);
        sprintf(sname, "config.[0].max_cities");
        config_lookup_int(cf, sname, &gtconfig.maxc);
        sprintf(sname, "config.[0].min_villages");
        config_lookup_int(cf, sname, &gtconfig.minv);
        sprintf(sname, "config.[0].max_villages");
        config_lookup_int(cf, sname, &gtconfig.maxv);
        sprintf(sname, "config.[0].dxsize");
        config_lookup_int(cf, sname, &gtconfig.dxsize);
        sprintf(sname, "config.[0].dysize");
        config_lookup_int(cf, sname, &gtconfig.dysize);
        sprintf(sname, "config.[0].compress_savefile");
        config_lookup_bool(cf, sname, &gtconfig.compress_savefile);

        printf(" OK\n");
        
        return 0;
}

int parse_data_files(int option)
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

        objid = 1;
        printf("Reading %s\n", MAIN_DATA_FILE);

        if(option == ONLY_CONFIG) {
                ret = parse_configfile();
                config_destroy(cf);
                return ret;
        }

        /* TODO:
         * This return value stuff makes rather little sense!!
         */
        ret = parse_configfile();
        ret = parse_objects();
        ret = parse_monsters();

        config_destroy(cf);

        ret = parse_roomdef_files();

        return ret;
}
