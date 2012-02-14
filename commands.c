/*
 * Gullible's Travails - 2011 Rewrite!
 *
 * Copyright 2011 Rolf Klausen
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#ifdef GT_USE_NCURSES
#include <curses.h>
#endif

#include "objects.h"
#include "actor.h"
#include "monsters.h"
#include "utils.h"
#include "datafiles.h"
#include "world.h"
#include "display.h"
#include "debug.h"
#include "saveload.h"
#include "commands.h"
#include "gt.h"

int numcommands;
cmd_t *curcommands;

cmd_t outsidecommands[] = {
        { 'j',       CMD_DOWN,        "Move down" },
        { KEY_DOWN,  CMD_DOWN,        "Move down" },
        { 'k',       CMD_UP,          "Move up" },
        { KEY_UP,    CMD_UP,          "Move up" },
        { 'h',       CMD_LEFT,        "Move left" },
        { KEY_LEFT,  CMD_LEFT,        "Move left" },
        { KEY_RIGHT, CMD_RIGHT,       "Move right" },
        { 'l',       CMD_RIGHT,       "Move right" },
        { 'y',       CMD_NW,          "Move up-left" },
        { 'u',       CMD_NE,          "Move up-right" },
        { 'b',       CMD_SW,          "Move down-left" },
        { 'n',       CMD_SE,          "Move down-right" },
        { 'q',       CMD_QUIT,        "Quit" },
        { 'i',       CMD_INVENTORY,   "Show inventory" },
        { 'w',       CMD_WIELDWEAR,   "Wield or wear an item" },
        { 'r',       CMD_UNWIELDWEAR, "Remove or unwield an item" },
        { KEY_F(5),  CMD_SAVE,        "Save" },
        { KEY_F(6),  CMD_LOAD,        "Load" },
        { ',',       CMD_PICKUP,      "Pick up something" },
        { '.',       CMD_REST,        "Rest one turn" },
        { '<',       CMD_ASCEND,      "Go up stairs" },
        { '>',       CMD_DESCEND,     "Go down stairs" },
#ifdef DEVELOPMENT_MODE
        { KEY_F(1),  CMD_WIZARDMODE,  "Toggle wizard mode" },
        { '+',       CMD_INCFOV,      "Increase FOV" },
        { '-',       CMD_DECFOV,      "Decrease FOV" },
        { 'f',       CMD_FLOODFILL,   "Floodfill (debug)" },
        { 's',       CMD_SPAWNMONSTER,"Spawn monster" },
        { KEY_NPAGE, CMD_LONGDOWN,    "" },
        { KEY_PPAGE, CMD_LONGUP,      "" },
        { 'K',       CMD_LONGUP,      "" },
        { 'J',       CMD_LONGDOWN,    "" },
        { 'H',       CMD_LONGLEFT,    "" },
        { 'L',       CMD_LONGRIGHT,   "" },
        { 'v',       CMD_TOGGLEFOV,   "Toggle FOV" },
        { KEY_F(4),  CMD_DUMPOBJECTS, "Dump objects" },
        { 'o',       CMD_DUMPOBJECTS, "" },
        { 'c',       CMD_DUMPCOLORS, "" },
        { 'p',       CMD_PATHFINDER, "" },
#endif
        //{ , CMD_WIELD, "Wield/wear" },
        //{ , CMD_IDENTIFYALL, "Identify everything" },
        //{ , CMD_DROP, "Drop something" },
        //{ , CMD_LOAD, "Load" },
        //{ , CMD_SKILLSCREEN, "Show skills" },
        //{ , CMD_QUIT, "Quit" },
};

int get_command()
{
        int key, i;

        key = gtgetch();
        if(key == 'q')
                return CMD_QUIT;       // easy exit even if C&C breaks down!

        for(i=0; i<numcommands; i++) {
                if(curcommands[i].key == key)
                        return curcommands[i].cmd;
        }

        return 0;
}

void init_commands()
{
        curcommands = outsidecommands;
        numcommands = (sizeof(outsidecommands) / sizeof(cmd_t));
}
