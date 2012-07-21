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
#include "o_effects.h"
#include "actor.h"
#include "monsters.h"
#include "quest.h"
#include "npc.h"
#include "world.h"
#include "datafiles.h"
#include "io.h"
#include "gt.h"
#include "utils.h"

// statistical debug stuff
int pluses, minuses;

// materials
int mats_bracelets[MATERIALS];
int mats_amulets[MATERIALS];
int mats_potions[POTS];

unsigned int oid_counter;
char objchars[] = {
        ' ',               // nothing
        '$',               // gold
        ')',               // weapon
        '[',               // armor
        'o',               // bracelet
        '"',               // amulet  (evt. 186!)
        '*',               // card    (evt. 246!)
        '/',               // wand
        '!',               // potion  (evt. 191)
};

char *materialstring[] = {
        0, "gold", "silver", "bronze", "copper", "wooden", "iron", "marble", "glass", "bone", "platinum", "steel", "blackwood", "brass", "ebony", "bloodwood", "stone"
};

char *potionstring[] = {
        0, "red", "green", "sparkling", "blue", "clear", "yellow", "pink", "amber", "golden orange",
        "orange", "lime green", "cyan", "sky blue", "violet", "crimson", "azure", "dark red", "sea green", "purple",
        "magenta", "fizzy", "cloudy"
};

obj_t get_objdef(int n)
{
        obj_t *tmp;

        tmp = objdefs->head->next;
        while(tmp->id != n) {

                if(tmp->next)
                        tmp = tmp->next;
                else
                        return *tmp;
        }

        return *tmp;
}

obj_t get_random_objdef_with_rarity(int rarity)
{
        obj_t *tmp;
        int i, j, foundone;

        foundone = false;

        while(!foundone) {
                j = 0;
                i = ri(0, game->objdefs);           // choose a random object

                tmp = objdefs->head->next;
                while(j != i) {                     // move up to that object  (should perhaps be a function of it's own, to look up objdef #x
                        if(tmp->next)
                                tmp = tmp->next;
                        j++;
                }

                if(tmp->rarity == rarity)           // if it has the requested rarity,
                        foundone = true;            // then we've found one, and can return it.
        }

        return *tmp;
}

obj_t *get_object_by_oid(inv_t *i, int oid)
{
        int j;

        for(j = 0; j < 52; j++) {
                if(i->object[j]) {
                        if(i->object[j]->oid == oid)
                                return i->object[j];;
                }
        }

        return 0;
}

obj_t *get_object_by_oid_from_masterlist(int oid)
{
        int i;

        for(i = 0; i < 2000; i++) {
                if(game->objects[i])
                        if(game->objects[i]->oid == oid)
                                return game->objects[i];
        }

        return 0;
}

int get_objdef_by_name(char *wanted)
{
        obj_t *o;

        o = objdefs->head->next;
        while(strcmp(o->basename, wanted))
                o = o->next;

        return o->id;
}

bool shall_autopickup(int type)
{
        int i, j;

        j = strlen(gtconfig.autopickup);
        for(i=0;i<j;i++)
                if(gtconfig.autopickup[i] == objchars[type])
                        return true;

        return false;
}

void start_autopickup()
{
        int i;

        for(i = 1; i <= OBJECT_TYPES; i++) {
                if(shall_autopickup(i))
                        gtconfig.ap[i] = true;
        }
}

void stop_autopickup()
{
        int i;

        for(i = 1; i <= OBJECT_TYPES; i++) {
                gtconfig.ap[i] = false;
        }
}

bool is_pair(obj_t *o)
{
        if(hasbit(o->flags, OF_GLOVES) || hasbit(o->flags, OF_FOOTWEAR))
                return true;
        else
                return false;
}

bool is_worn(obj_t *o)      // worn by player, that is.. or wielded (weapons)
{
        int i;

        if(o == player->weapon)
                return true;

        for(i=0;i<WEAR_SLOTS;i++) {
                if(o == player->w[i])
                        return true;
        }

        return false;
}

void add_to_master_object_list(obj_t *o)
{
        int i;

        for(i = 0; i < 2000; i++) {
                if(game->objects[i] == NULL) {
                        game->objects[i] = o;
                        game->num_objects++;
                        return;
                }
        }
}

void remove_from_master_object_list(obj_t *o)
{
        int i;

        for(i = 0; i < 2000; i++) {
                if(game->objects[i] == o) {
                        game->objects[i] = NULL;
                        game->num_objects--;
                        return;
                }
        }
}

void clear_master_object_list()
{
        int i;

        for(i = 0; i < 2000; i++)
                game->objects[i] = NULL;
}

void unspawn_object(obj_t *m)
{
        if(m) {
                remove_from_master_object_list(m);
                gtfree(m);
        }
}

char *get_enchanted_description()
{
        char *n;
        int i;

        n = malloc(sizeof(char) * 100);
        i = dice(1, 7, 0);
        switch(i) {
                case 1: sprintf(n, "nicely decorated "); break;
                case 2: sprintf(n, "finely engraved "); break;
                case 3: sprintf(n, "polished "); break;
                case 4: sprintf(n, "radiant "); break;
                case 5: sprintf(n, "strangely inscribed "); break;
                case 6: sprintf(n, "shiny "); break;
                default: n[0] = '\0'; break;
        }

        return n;
}

/*
 * Generate the full name of object
 */

void generate_fullname(obj_t *o)
{
        char *n;
        int i;

        n = gtmalloc(sizeof(char) * 250);
        if(o->type == OT_WEAPON) {
                if(!is_identified(o) && (o->attackmod || o->damagemod || (o->damagemod && o->attackmod))) {
                        strcpy(n, get_enchanted_description());
                        strcat(n, o->basename);
                } else {
                        if(!o->attackmod && !o->damagemod) {
                                strcat(n, o->basename);
                        }

                        if(o->attackmod && !o->damagemod) {
                                if(is_identified(o)) {
                                        if(o->attackmod > 0)
                                                sprintf(n, "+%d,+0", o->attackmod);
                                        if(o->attackmod < 0)
                                                sprintf(n,  "%d,+0", o->attackmod);
                                } else {
                                }
                                strcat(n, " ");
                                strcat(n, o->basename);
                        }
                        if(is_identified(o) && !o->attackmod && o->damagemod) {
                                if(o->damagemod > 0)
                                        sprintf(n, "0,+%d", o->damagemod);
                                if(o->damagemod < 0)
                                        sprintf(n, "0,%d", o->damagemod);
                                strcat(n, " ");
                                strcat(n, o->basename);
                        }
                        if(is_identified(o) && o->attackmod && o->damagemod) {
                                if(o->attackmod > 0)
                                        sprintf(n, "+%d", o->attackmod);
                                if(o->attackmod < 0)
                                        sprintf(n,  "%d", o->attackmod);
                                if(o->damagemod > 0)
                                        sprintf(n, "%s,+%d", n, o->damagemod);
                                if(o->damagemod < 0)
                                        sprintf(n, "%s,%d", n, o->damagemod);
                                strcat(n, " ");
                                strcat(n, o->basename);
                        }
                }
        } else if(o->type == OT_ARMOR) {
                if(is_identified(o)) {
                        if(o->attackmod > 0)
                                sprintf(n, "+%d ", o->attackmod);

                        if(o->attackmod < 0)
                                sprintf(n,  "%d ", o->attackmod);

                        if(o->effects) {
                                if(strlen(o->fullname))
                                        sprintf(n, "%s%s [", n, o->truename);
                                else
                                        sprintf(n, "%s%s [", n, o->basename);

                                for(i=0;i<MAX_EFFECTS;i++) {
                                        if(o->effect[i].effect) {
                                                switch(o->effect[i].effect) {
                                                        case OE_DEXTERITY: sprintf(n, "%sDex+%d", n, o->effect[i].gain); break;
                                                        case OE_STRENGTH:  sprintf(n, "%sStr+%d", n, o->effect[i].gain); break;
                                                        case OE_WISDOM:    sprintf(n, "%sWis+%d", n, o->effect[i].gain); break;
                                                        case OE_INTELLIGENCE: sprintf(n, "%sInt+%d", n, o->effect[i].gain); break;
                                                        case OE_PHYSIQUE:  sprintf(n, "%sPhy+%d", n, o->effect[i].gain); break;
                                                        case OE_CHARISMA:  sprintf(n, "%sCha+%d", n, o->effect[i].gain); break;
                                                        case OE_SPEED: strcat(n, "Speed"); break;
                                                }
                                                if(i < MAX_EFFECTS-1 && o->effect[i+1].effect)
                                                        strcat(n, ", ");
                                        }
                                }
                                strcat(n, "]");
                        } else
                                strcat(n, o->basename); 

                }
                
                if((!is_identified(o) && o->attackmod)) {
                        strcpy(n, get_enchanted_description());
                        strcat(n, o->basename); 
                }

                if((!is_identified(o) && o->effects)) {
                        strcpy(n, get_enchanted_description());
                        strcat(n, o->basename); 
                }

                if(!is_identified(o) && !o->attackmod && !o->effects) {
                        strcat(n, o->basename); 
                }
        } else if(o->type == OT_BRACELET) {
                if(is_identified(o) && is_id_mod(o)) {
                        if(o->attackmod > 0)
                                sprintf(n, "+%d ", o->attackmod);
                        if(o->attackmod < 0)
                                sprintf(n,  "%d ", o->attackmod);
                        strcat(n, o->basename); 
                } else if(is_identified(o) && !is_id_mod(o)) {
                        sprintf(n, "%s", o->basename);
                } else if(!is_identified(o)) {
                        sprintf(n, "%s bracelet", materialstring[(int)o->material]);
                }
        } else if(o->type == OT_AMULET) {
                if(is_identified(o)) {
                        sprintf(n, "%s", o->basename);
                } else {
                        sprintf(n, "%s amulet", materialstring[(int)o->material]);
                }
        } else if(o->type == OT_POTION) {
                if(is_identified(o)) {
                        sprintf(n, "%s", o->basename);
                } else {
                        sprintf(n, "%s potion", potionstring[(int)o->material]);
                }
        } else {
                strcat(n, o->basename);
        }

        if(hasbit(o->flags, OF_HOLYFUCK))
                strcat(n, " of Holy Fuck");
        
        //fprintf(stderr, "generated name %s\n", n);
        strcpy(o->fullname, n);
        strcpy(o->displayname, n);
}

/*
 * place a spawned object at (y,x)
 */
bool place_object_at(obj_t *obj, int y, int x, void *p)
{
        level_t *l;

        l = (level_t *) p;
        
        if(l->c[y][x].type == AREA_PLAIN || l->c[y][x].type == DNG_FLOOR) {
                if(!l->c[y][x].inventory)
                        l->c[y][x].inventory = init_inventory();

                if(move_to_inventory(obj, l->c[y][x].inventory))
                        return true;
        } else
                return false;

        return false;
}

void add_attackmod(obj_t *new, void *level)
{
        level_t *l;
        int lev;

        l = (level_t *) level;
        if(l)
                lev = l->level;
        else
                lev = 1;

        if(!is_unique(new) && (is_weapon(new) || is_armor(new) || is_bracelet(new))) {
                if(new->type == OT_BRACELET)
                        while(!new->attackmod)
                                new->attackmod = ri(-1, 1);

                if(perc(50+(player->level*2))) {
                        if(perc(40)) {                // a bracelet must be at least +/- 1 (or 0 for malfunctioning bracelets!)
                                if(perc(66))
                                        new->attackmod = ri(0, 1);
                                else
                                        new->attackmod = ri(-1, 0);
                        }
                        if(perc(30 + (lev*3)) && !new->attackmod) {
                                if(perc(66))
                                        new->attackmod = ri(1, 2);
                                else
                                        new->attackmod = ri(-2, -1);
                        }
                        if(perc(20 + (lev*3)) && !new->attackmod) {
                                if(perc(66))
                                        new->attackmod = ri(1, 3);
                                else
                                        new->attackmod = ri(-3, -1);
                        }
                        if(perc(10 + (lev*3)) && !new->attackmod) {
                                if(perc(50 + (lev*2))) {
                                        if(perc(66))
                                                new->attackmod = ri(1, 4);
                                        else
                                                new->attackmod = ri(-4, -1);
                                } else {
                                        if(perc(66))
                                                new->attackmod = ri(1, 5);
                                        else
                                                new->attackmod = ri(-5, -1);
                                }
                        }
                }
        }

        if(new->attackmod && (is_weapon(new) || is_armor(new))) {
                clearbit(new->flags, OF_IDENTIFIED);
                setbit(new->flags, OF_OBVIOUS);                    // TODO: change to some kind of ID system for weapons! (and armor?)
                new->color = COLOR_BLUE;
        }
}

void add_damagemod(obj_t *new, void *level)
{
        level_t *l;
        int lev;

        l = (level_t *) level;
        if(l)
                lev = l->level;
        else
                lev = 1;

        if(!is_unique(new) && is_weapon(new)) {
                if(perc(60+(player->level*2))) {
                        if(perc(40)) {
                                if(perc(66))
                                        new->damagemod = ri(0, 1);
                                else
                                        new->damagemod = ri(-1, 0);
                        }
                        if(perc(30 + (lev*3)) && !new->damagemod) {
                                if(perc(66))
                                        new->damagemod = ri(1, 2);
                                else
                                        new->damagemod = ri(-2, -1);
                        }
                        if(perc(20 + (lev*3)) && !new->damagemod) {
                                if(perc(66))
                                        new->damagemod = ri(1, 3);
                                else
                                        new->damagemod = ri(-3, -1);
                        }
                        if(perc(10 + (lev*3)) && !new->damagemod) {
                                if(perc(50 + (lev*2))) {
                                        if(perc(66))
                                                new->damagemod = ri(1, 4);
                                        else
                                                new->damagemod = ri(-4, -1);
                                } else {
                                        if(perc(66))
                                                new->damagemod = ri(1, 5);
                                        else
                                                new->damagemod = ri(-5, -1);
                                }
                        }
                }
        }

        if(new->damagemod && is_weapon(new)) {
                clearbit(new->flags, OF_IDENTIFIED);
                setbit(new->flags, OF_OBVIOUS);
                new->color = COLOR_BLUE;
        }
}

obj_t *spawn_object(int n, void *level)
{
        obj_t *new;

        new = gtmalloc(sizeof(obj_t));
        if(!new)
                return false;
        
        *new = get_objdef(n);

        oid_counter++;
        new->oid = oid_counter;

        // maybe this object is magical?
        add_attackmod(new, level);
        add_damagemod(new, level);

        generate_fullname(new);
        new->quantity = 1;               // TODO: make it possible to spawn multiple stackable objects.
        add_to_master_object_list(new);

        return new;
}

obj_t *spawn_object_with_rarity(int rarity, void *level)
{
        obj_t *new;

        new = gtmalloc(sizeof(obj_t));
        if(!new)
                return false;
        
        *new = get_random_objdef_with_rarity(rarity);

        oid_counter++;
        new->oid = oid_counter;

        // maybe this object is magical?
        add_attackmod(new, level);
        add_damagemod(new, level);

        generate_fullname(new);
        new->quantity = 1;               // TODO: make it possible to spawn multiple stackable objects.
        add_to_master_object_list(new);

        return new;
}

/*
 * spawn an object (objdef n) and place it at (y,x) on level
 */
bool spawn_object_at(int y, int x, int n, void *level)
{
        obj_t *o;

        o = spawn_object(n, level);
        if(!o)
                return false;

        if(!place_object_at(o, y, x, level)) {
                unspawn_object(o);
                return false;
        }

        return true;
}

bool spawn_object_with_rarity_at(int y, int x, int rarity, void *level)
{
        obj_t *o;

        o = spawn_object_with_rarity(rarity, level);
        if(!o)
                return false;

        if(!place_object_at(o, y, x, level)) {
                unspawn_object(o);
                return false;
        }

        generate_fullname(o);
        return true;
}

/*
 * Spawn num objects on level l
 */
void spawn_objects(int num, void *p)
{
        int i, x, y;
        double f;
        int verycommon, common, uncommon, rare, veryrare;
        level_t *l;

        /* 
         * To deal with rarity we'll try this:
         * COMMON % of num will be, well, COMMON
         * UNCOMMON % uncommon
         * etc.
         */

        f = (double) ((float)num / 100.0f) * (float)VERYCOMMON;
        verycommon = round(f);
        f = (double) ((float)num / 100.0f) * (float)COMMON;
        common = round(f);
        f = (double) ((float)num / 100.0f) * (float)UNCOMMON;
        uncommon = round(f);
        f = (double) ((float)num / 100.0f) * (float)RARE;
        rare = round(f);
        f = (double) ((float)num / 100.0f) * (float)VERYRARE;
        veryrare = round(f);

        //printf("Spawning %d objects:\n%d very common\n%d common\n%d uncommon\n%d rare\n%d very rare\ntotal %d\n", num, verycommon, common, uncommon, rare, veryrare, common + uncommon + rare + veryrare);

        i = 0;
        l = (level_t *) p;
        verycommon = common = uncommon = rare = veryrare = 0;

        while(i < num) {
                int p, rarity;

                x = ri(1, l->xsize-1);
                y = ri(1, l->ysize-1);

                p = ri(1, 100);
                if(p <= VERYRARE)
                        rarity = VERYRARE;
                else if(p > VERYRARE && p <= (VERYRARE + RARE))
                        rarity = RARE;
                else if(p > (VERYRARE + RARE) && p <= (VERYRARE + RARE + UNCOMMON))
                        rarity = UNCOMMON;
                else if(p > (VERYRARE + RARE + UNCOMMON) && p <= (VERYRARE + RARE + UNCOMMON + COMMON))
                        rarity = COMMON;
                else if(p > (VERYRARE + RARE + UNCOMMON + COMMON) && p <= (VERYRARE + RARE + UNCOMMON + COMMON + VERYCOMMON))
                        rarity = VERYCOMMON;
                
                if (spawn_object_with_rarity_at(y, x, rarity, l)) {
                        i++;
                        switch(rarity) {
                                case VERYCOMMON: verycommon++;
                                case COMMON: common++; break;
                                case UNCOMMON: uncommon++; break;
                                case RARE: rare++; break;
                                case VERYRARE: veryrare++;
                        };
                }
                        
        }

        //printf("Generated:\n\ttotal:\t%d\n\tvery common:\t%d\n\tcommon:\t%d\n\tuncommon:\t%d\n\trare:\t%d\n\tveryrare:\t%d\n\n", i, verycommon, common, uncommon, rare, veryrare);
}

void spawn_gold(int n, inv_t *i)
{
        if(!i)
                i = init_inventory();

        i->gold = n;
}

bool spawn_gold_at(int y, int x, int n, void *level)
{
        level_t *l;
        inv_t *i;

        l = (level_t *) level;
        i = l->c[y][x].inventory;

        if(l->c[y][x].type == AREA_PLAIN || l->c[y][x].type == DNG_FLOOR) {
                if(!i)
                        i = init_inventory();

                i->gold = n;
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
                if (spawn_gold_at(y, x, n, l)) {
                        i++;
                }
        }

}

/*
 * Spawn a total of maxtotal gold on level p.
 * The gold will be evenly/randomly distributed on the level.
 */
void spawn_gold_with_maxtotal(int maxtotal, void *p)
{
        int i, x, y, n;
        level_t *l;

        i = maxtotal;
        l = (level_t *) p;
        while(i) {
                x = ri(1, l->xsize-1);
                y = ri(1, l->ysize-1);
                n = ri(1, l->level*10);
                if(n > i)
                        n = i;
                if (spawn_gold_at(y, x, n, l)) {
                        i -= n;
                }
        }
}

// identify all objects which are the same material & type as o
void do_identify_all_of_type(obj_t *o)
{
        int i;

        if(!is_weapon(o) && !is_armor(o)) {
                for(i = 0; i < 2000; i++) {
                        if(game->objects[i]) {
                                if((game->objects[i]->type == o->type) && (game->objects[i]->material == o->material)) {
                                        setbit(game->objects[i]->flags, OF_IDENTIFIED);
                                        generate_fullname(game->objects[i]);
                                }
                        }
                }
        }
}

/*
 * Functions related to actors and interacting with objects
 */
inv_t *init_inventory()
{
        inv_t *i;

        i = gtmalloc(sizeof(inv_t));

        return i;
}

void puton(void *a, int slot, obj_t *o)
{
        actor_t *actor;

        actor = (actor_t *) a;

        youc(COLOR_INFO, "put on %s.", a_an(pair(o)));
        actor->w[slot] = o;
        apply_effects(actor, o);

        if(is_armor(o)) {
                actor->ac += o->ac;
                actor->ac += o->attackmod;
        }
        
        if(actor == player) {
                if(hasbit(o->flags, OF_IDENTIFIED)) {
                        setbit(o->flags, OF_ID_MOD);
                        generate_fullname(o);
                }

                if(!hasbit(o->flags, OF_IDENTIFIED) && hasbit(o->flags, OF_OBVIOUS)) {
                        setbit(o->flags, OF_IDENTIFIED);
                        setbit(o->flags, OF_ID_MOD);
                        do_identify_all_of_type(o);
                        generate_fullname(o);
                        strcpy(o->displayname, o->fullname);
                        gtprintfc(COLOR_INFO, "This is %s!", a_an(pair(o)));
                }
        }
}

void identify(obj_t *o)
{
        setbit(o->flags, OF_IDENTIFIED);
        do_identify_all_of_type(o);
        generate_fullname(o);
        strcpy(o->displayname, o->fullname);
}

void quaff(void *a, obj_t *o)
{
        int slot;
        actor_t *actor;

        actor = (actor_t *) a;

        if(is_potion(o)) {
                apply_effects(actor, o);

                if((actor == player) && !hasbit(o->flags, OF_IDENTIFIED) && hasbit(o->flags, OF_OBVIOUS)) {
                        identify(o);
                        gtprintfc(COLOR_INFO, "That was %s!", a_an(pair(o)));
                }

                slot = object_to_slot(o, actor->inventory);
                actor->inventory->object[slot]->quantity--;

                if(actor->inventory->object[slot]->quantity < 1) {
                        actor->inventory->object[slot] = NULL;
                        actor->inventory->num_used--;
                } 
        }
}

void wear(void *a, obj_t *o)
{
        actor_t *actor;

        actor = (actor_t *) a;

        if(actor == player) {
                if(is_bracelet(o)) {
                        char c;

                        c = ask_for_hand();
                        if(!c) {
                                gtprintf("Alright then.");
                                return;
                        }
                        if(c == 'l') {
                                if(pw_leftbracelet) {
                                        if(!yesno("Do you want to remove your %s", pw_leftbracelet->displayname)) {
                                                gtprintf("OK then.");
                                                return;
                                        } else {
                                                unwear(actor, pw_leftbracelet);
                                                puton(actor, SLOT_LEFTBRACELET, o);
                                        }
                                } else {
                                        puton(actor, SLOT_LEFTBRACELET, o);
                                }
                        } else if(c == 'r') {
                                if(pw_rightbracelet) {
                                        if(!yesno("Do you want to remove your %s", pw_rightbracelet->displayname)) {
                                                gtprintf("OK then.");
                                                return;
                                        } else {
                                                unwear(actor, pw_rightbracelet);
                                                puton(actor, SLOT_RIGHTBRACELET, o);
                                        }
                                } else {
                                        puton(actor, SLOT_RIGHTBRACELET, o);
                                }
                        }

                        if(!o->attackmod)
                                gtprintfc(COLOR_INFO, "The %s seems to be malfunctioning!", o->displayname);      // change this when we implement the identification system!

                }

                if(is_amulet(o)) {
                        if(pw_amulet) {
                                if(!yesno("Do you want to remove your %s", pw_amulet->displayname)) {
                                        gtprintf("OK then.");
                                        return;
                                } else {
                                        unwear(actor, pw_amulet);
                                        puton(actor, SLOT_AMULET, o);
                                }
                        } else {
                                puton(actor, SLOT_AMULET, o);
                        }
                }

                if(is_armor(o)) {
                        if(is_bodyarmor(o)) {
                                if(pw_body) {
                                        if(!yesno("Do you want to remove your %s", pw_body->displayname)) {
                                                gtprintf("OK then.");
                                                return;
                                        } else {
                                                unwear(actor, pw_body);
                                                puton(actor, SLOT_BODY, o);
                                        }
                                } else {
                                        puton(actor, SLOT_BODY, o);
                                }
                        }

                        if(is_headgear(o)) {
                                if(pw_headgear) {
                                        if(!yesno("Do you want to remove your %s", pw_headgear->displayname)) {
                                                gtprintf("OK then.");
                                                return;
                                        } else {
                                                unwear(actor, pw_headgear);
                                                puton(actor, SLOT_HEAD, o);
                                        }
                                } else {
                                        puton(actor, SLOT_HEAD, o);
                                }
                        }

                        if(is_footwear(o)) {
                                if(pw_footwear) {
                                        if(!yesno("Do you want to remove your %s", pw_footwear->displayname)) {
                                                gtprintf("OK then.");
                                                return;
                                        } else {
                                                unwear(actor, pw_footwear);
                                                puton(actor, SLOT_FOOTWEAR, o);
                                        }
                                } else {
                                        puton(actor, SLOT_FOOTWEAR, o);
                                }
                        }

                        if(is_gloves(o)) {
                                if(pw_gloves) {
                                        if(!yesno("Do you want to remove your %s", pw_gloves->displayname)) {
                                                gtprintf("OK then.");
                                                return;
                                        } else {
                                                unwear(actor, pw_gloves);
                                                puton(actor, SLOT_GLOVES, o);
                                        }
                                } else {
                                        puton(actor, SLOT_GLOVES, o);
                                }
                        }

                        if(is_shield(o)) {
                                if(pw_shield) {
                                        if(!yesno("Do you want to remove your %s", pw_shield->displayname)) {
                                                gtprintf("OK then.");
                                                return;
                                        } else {
                                                unwear(actor, pw_shield);
                                                puton(actor, SLOT_SHIELD, o);
                                        }
                                } else {
                                        puton(actor, SLOT_SHIELD, o);
                                }
                        }
                }
        }
}

void wield(void *a, obj_t *o)
{
        actor_t *actor;

        actor = (actor_t *) a;
        player->weapon = o;
        apply_effects(actor, o);
        if((actor == player) && !hasbit(o->flags, OF_IDENTIFIED) && hasbit(o->flags, OF_OBVIOUS)) {             // TODO: Fix so taht identifying weapons/armor is based on e.g. experience with said type of weapon.
                identify(o);
                gtprintfc(COLOR_INFO, "You believe this is %s!", a_an(pair(o)));
        }
}

void wieldwear(void *a, obj_t *o)
{
        actor_t *actor;

        actor = (actor_t *) a;
        if(!o)
                return;

        if(actor == player) {
                if(is_worn(o)) {
                        if(is_weapon(o))
                                youc(COLOR_INFO, "are already wielding that!");
                        else
                                youc(COLOR_INFO, "are already wearing that!");
                        return;
                }

                if(is_weapon(o)) {
                        wield(actor, o);
                        youc(COLOR_INFO, "are now wielding a %s.", o->displayname);
                        //player->ticks -= TICKS_WIELDWEAR;
                } else {
                        wear(actor, o);
                        //player->ticks -= TICKS_WIELDWEAR;
                }
        }
}

void unwear(void *a, obj_t *o)
{
        int i;
        actor_t *actor;

        actor = (actor_t *) a;

        for(i = 0; i < WEAR_SLOTS; i++) {
                if(actor->w[i] == o) {
                        youc(COLOR_INFO, "remove your %s.", o->displayname);
                        actor->w[i] = NULL;
                        unapply_effects(actor, o);
                        if(is_armor(o)) {
                                actor->ac -= o->ac;
                                actor->ac -= o->attackmod;
                        }
                }
        }
}

void unwield(void *a, obj_t *o)
{
        actor_t *actor;

        actor = (actor_t *) a;
        actor->weapon = NULL;
        unapply_effects(actor, o);
}

void unwieldwear(void *a, obj_t *o)
{
        actor_t *actor;

        actor = (actor_t *) a;
        if(!is_worn(o) && (actor == player)) {
                if(is_weapon(o))
                        youc(COLOR_INFO, "aren't wielding that!");
                else
                        youc(COLOR_INFO, "aren't wearing that!");
                return;
        }

        if(is_weapon(o)) {
                unwield(actor, o);
                youc(COLOR_INFO, "unwield the %s.", o->displayname);
        } else {
                unwear(actor, o);
        }
}

int get_first_free_slot_in_inventory(inv_t *i)
{
        int j;

        j = 0;
        while(i->object[j])
                j++;

        return j;
}

int get_first_used_slot(inv_t *i)
{
        int j;

        for(j = 0; j < 52; j++) {
                if(i->object[j])
                        return j;
        }

        return -1;
}

int get_next_used_slot_after(int n, inv_t *i)
{
        int j;

        for(j = (n+1); j < 52; j++) {
                if(i->object[j])
                        return j;
        }

        return -1;
}

/**
 * @brief Check if two objects are identical.
 *
 * We'll probably want to expand this one later.
 *
 * @param a First object.
 * @param b Second object.
 *
 * @return true if objects are identical, false if not.
 */
bool objects_are_identical(obj_t *a, obj_t *b)
{
        if(!strcmp(a->fullname, b->fullname))
                return true;
        else
                return false;
}

bool has_identical_object(inv_t *i, obj_t *o)
{
        int j;

        for(j = 0; j < 52; j++) {
                if(i->object[j]) {
                        if(objects_are_identical(o, i->object[j])) {
                                return true;
                        }
                }
        }

        return false;
}

int get_slot_of_object(inv_t *i, obj_t *o)
{
        int j;

        for(j = 0; j < 52; j++) {
                if(i->object[j]) {
                        if(objects_are_identical(o, i->object[j])) {
                                return j;
                        }
                }
        }

        return -1;
}

void pick_up(obj_t *o, void *p)
{
        actor_t *a;
        int slot;

        a = (actor_t *) p;

        if(!a->inventory)
                a->inventory = init_inventory();

        if(!is_stackable(o)) {
                slot = get_first_free_slot_in_inventory(a->inventory);
                a->inventory->object[slot] = o;
                a->inventory->num_used++;
        } else {
                if(has_identical_object(a->inventory, o)) {
                        slot = get_slot_of_object(a->inventory, o);
                        a->inventory->object[slot]->quantity += o->quantity;
                } else {
                        slot = get_first_free_slot_in_inventory(a->inventory);
                        a->inventory->object[slot] = o;
                        a->inventory->num_used++;
                }
        }

        gtprintfc(COLOR_WHITE, "  %c - %s", slot_to_letter(slot), a_an(pair(o)));
}

void place_object_in_cell(obj_t *o, cell_t *c)
{
        int slot;

        if(!c->inventory)
               c->inventory = init_inventory();

        slot = get_first_free_slot_in_inventory(c->inventory);
        c->inventory->object[slot] = o;
        c->inventory->num_used++;
}

void drop(void *actor, obj_t *o)
{
        int slot;
        actor_t *a;

        a = (actor_t *) actor;

        if(a == player && is_worn(o))
                unwieldwear(a, o);

        place_object_in_cell(o, &world->curlevel->c[a->y][a->x]);
        setbit(o->flags, OF_DONOTAP);   // Don't autopickup a dropped item. TODO: Make this configurable?!

        slot = object_to_slot(o, a->inventory);
        a->inventory->object[slot] = NULL;
        a->inventory->num_used--;
}

/*
 * Move object *o to first free slot in inventory *i,
 * or stack it if appropriate.
 */
bool move_to_inventory(obj_t *o, inv_t *i)
{
        if(!i)
                i = init_inventory();

        if(o->type == OT_GOLD) {
                i->gold += o->quantity;
                unspawn_object(o);
                return true;
        } else {
                int x;

                if(!is_stackable(o)) {
                        x = get_first_free_slot_in_inventory(i);
                        i->object[x] = o;
                        i->num_used++;
                } else {
                        if(has_identical_object(i, o)) {
                                x = get_slot_of_object(i, o);
                                i->object[x]->quantity += o->quantity;
                        } else {
                                x = get_first_free_slot_in_inventory(i);
                                i->object[x] = o;
                                i->num_used++;
                        }
                }

                return true;
        }

        return false;
}

void init_objects()
{
        int i, j;

        mats_bracelets[0] = mats_amulets[0] = mats_potions[0] = 0;

        for(i = 0; i <= MATERIALS; i++) {
                j = ri(1, MATERIALS);
                while(mats_bracelets[j] != 0)
                        j = ri(1, MATERIALS);
                mats_bracelets[j] = i;
        }

        for(i = 0; i <= MATERIALS; i++) {
                j = ri(1, MATERIALS);
                while(mats_amulets[j] != 0)
                        j = ri(1, MATERIALS);
                mats_amulets[j] = i;

        }

        for(i = 0; i <= POTS; i++) {
                j = ri(1, POTS);
                while(mats_potions[j] != 0)
                        j = ri(1, POTS);
                mats_potions[j] = i;
        }
}
