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
#include "you.h"
#include "display.h"
#include "debug.h"
#include "saveload.h"
#include "commands.h"
#include "gt.h"

int numcommands;
cmd_t *curcommands;

cmd_t outsidecommands[] = {
        { 'j', CMD_DOWN, "Move down" },
        { KEY_DOWN, CMD_DOWN, "Move down" },
        { 'k', CMD_UP, "Move up" },
        { KEY_UP, CMD_UP, "Move up" },
        { 'h', CMD_LEFT, "Move left" },
        { KEY_LEFT, CMD_LEFT, "Move left" },
        { 'k', CMD_RIGHT, "Move right" },
        { KEY_RIGHT, CMD_RIGHT, "Move right" },
        { 'l', CMD_RIGHT, "Move right" },
        { 'y', CMD_NW, "Move up-left" },
        { 'u', CMD_NE, "Move up-right" },
        { 'b', CMD_SW, "Move down-left" },
        { 'n', CMD_SE, "Move down-right" },
        //{ , CMD_PICKUP, "Pick up something" },
        //{ , CMD_DROP, "Drop something" },
        { 'q', CMD_QUIT, "Quit" },
        //{ , CMD_QUIT, "Quit" },
        { 'K', CMD_LONGUP, "" },
        { 'J', CMD_LONGDOWN, "" },
        { 'H', CMD_LONGLEFT, "" },
        { 'L', CMD_LONGRIGHT, "" },
        { 'v', CMD_TOGGLEFOV, "Toggle FOV" },
        { KEY_F(5), CMD_SAVE, "Save" },
        { KEY_F(6), CMD_LOAD, "Load" },
        //{ , CMD_LOAD, "Load" },
        //{ , CMD_SKILLSCREEN, "Show skills" },
        { 'd', CMD_ENTERDUNGEON, "Enter dungeon" },
        { 'f', CMD_FLOODFILL, "Floodfill (debug)" },
        { 's', CMD_SPAWNMONSTER, "Spawn monster" },
        { 'w', CMD_WIZARDMODE, "Toggle wizard mode" },
        //{ , CMD_WIELD, "Wield/wear" },
        //{ , CMD_IDENTIFYALL, "Identify everything" },
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
