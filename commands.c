/*
 * Dark Shadows - The Roguelike
 *
 * Copyright 2011 Rolf Klausen
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <libtcod/libtcod.h>

#include "objects.h"
#include "actor.h"
#include "monsters.h"
#include "utils.h"
#include "world.h"
#include "datafiles.h"
#include "display.h"
#include "debug.h"
#include "saveload.h"
#include "commands.h"
#include "dstime.h"
#include "dsrl.h"

int numcommands;
cmd_t *curcommands;

#ifdef DS_USE_NCURSES
#include <curses.h>

cmd_t outsidecommands[] = {
        { 'j',       CMD_DOWN,        "Move down" },
        { 50,        CMD_DOWN,        "Move down" },
        { KEY_DOWN,  CMD_DOWN,        "Move down" },
        { 'k',       CMD_UP,          "Move up" },
        { 56,        CMD_UP,          "Move up" },
        { KEY_UP,    CMD_UP,          "Move up" },
        { 'h',       CMD_LEFT,        "Move left" },
        { 52,        CMD_LEFT,        "Move left" },
        { KEY_LEFT,  CMD_LEFT,        "Move left" },
        { 'l',       CMD_RIGHT,       "Move right" },
        { 54,        CMD_RIGHT,       "Move right" },
        { KEY_RIGHT, CMD_RIGHT,       "Move right" },
        { 'y',       CMD_NW,          "Move up-left" },
        { 55,        CMD_NW,          "Move up-left" },
        { 'u',       CMD_NE,          "Move up-right" },
        { 57,        CMD_NE,          "Move up-right" },
        { 'b',       CMD_SW,          "Move down-left" },
        { 49,        CMD_SW,          "Move down-left" },
        { 'n',       CMD_SE,          "Move down-right" },
        { 51,        CMD_SE,          "Move down-right" },
        { 27,        CMD_QUIT,        "Quit" },
        { 'i',       CMD_INVENTORY,   "Show inventory" },
        { 'w',       CMD_WIELDWEAR,   "Wield or wear an item" },
        { 'r',       CMD_UNWIELDWEAR, "Remove or unwield an item" },
        { KEY_F(5),  CMD_SAVE,        "Save" },
        { KEY_F(6),  CMD_LOAD,        "Load" },
        { ',',       CMD_PICKUP,      "Pick up something" },
        { '.',       CMD_REST,        "Rest one turn" },
        { '<',       CMD_ASCEND,      "Go up stairs" },
        { '>',       CMD_DESCEND,     "Go down stairs" },
        { 'd',       CMD_DROP,        "Drop an object" },
#ifdef DEVELOPMENT_MODE
        { KEY_F(1),  CMD_WIZARDMODE,  "Toggle wizard mode" },
        { KEY_F(2),  CMD_INCTIME,     "Time travel!?" },
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
        //{ , CMD_IDENTIFYALL, "Identify everything" },
        //{ , CMD_SKILLSCREEN, "Show skills" },
};

int get_command()
{
        int key, i;

        key = dsgetch();
        if(key == 27)
                return CMD_QUIT;       // easy exit even if C&C breaks down!

        if(key == ERR)
                return CMD_MOVE_ON;

        for(i=0; i<numcommands; i++) {
                if(curcommands[i].key == key)
                        return curcommands[i].cmd;
        }

        dsprintf("unknown key: %d", key);

        return 0;
}
#endif

#ifdef DS_USE_LIBTCOD

#include <libtcod/libtcod.h>

cmd_t outsidecommands[] = {
        { 'j',       CMD_DOWN,        "Move down" },
        { 'k',       CMD_UP,          "Move up" },
        { 'h',       CMD_LEFT,        "Move left" },
        { 'l',       CMD_RIGHT,       "Move right" },
        { 'y',       CMD_NW,          "Move up-left" },
        { 'u',       CMD_NE,          "Move up-right" },
        { 'b',       CMD_SW,          "Move down-left" },
        { 'n',       CMD_SE,          "Move down-right" },
        { 'i',       CMD_INVENTORY,   "Show inventory" },
        { 'w',       CMD_WIELDWEAR,   "Wield or wear an item" },
        { 'r',       CMD_UNWIELDWEAR, "Remove or unwield an item" },
        //{ TCODK_F5,  CMD_SAVE,        "Save" },
        //{ TCODK_F6,  CMD_LOAD,        "Load" },
        { ',',       CMD_PICKUP,      "Pick up something" },
        { '.',       CMD_REST,        "Rest one turn" },
        { '<',       CMD_ASCEND,      "Go up stairs" },
        { '>',       CMD_DESCEND,     "Go down stairs" },
        { 'd',       CMD_DROP,        "Drop an object" },
#ifdef DEVELOPMENT_MODE
        { 'W',       CMD_WIZARDMODE,  "Toggle wizard mode" },
        //{ TCODK_F2,  CMD_INCTIME,     "Time travel!?" },
        { '+',       CMD_INCFOV,      "Increase FOV" },
        { '-',       CMD_DECFOV,      "Decrease FOV" },
        { 'f',       CMD_FLOODFILL,   "Floodfill (debug)" },
        { 's',       CMD_SPAWNMONSTER,"Spawn monster" },
        //{ TCODK_PAGEDOWN, CMD_LONGDOWN,    "" },
        //{ TCODK_PAGEUP, CMD_LONGUP,      "" },
        { 'K',       CMD_LONGUP,      "" },
        { 'J',       CMD_LONGDOWN,    "" },
        { 'H',       CMD_LONGLEFT,    "" },
        { 'L',       CMD_LONGRIGHT,   "" },
        { 'v',       CMD_TOGGLEFOV,   "Toggle FOV" },
        //{ KEY_F(4),  CMD_DUMPOBJECTS, "Dump objects" },
        { 'o',       CMD_DUMPOBJECTS, "" },
        { 'c',       CMD_DUMPCOLORS, "" },
        { 'p',       CMD_PATHFINDER, "" },
#endif
        //{ , CMD_IDENTIFYALL, "Identify everything" },
        //{ , CMD_SKILLSCREEN, "Show skills" },
};

#endif

int get_command()
{
        int i;
        TCOD_key_t key;

        TCOD_console_flush();
        key = dsgetch();
        if(key.vk == TCODK_ESCAPE)
                return CMD_QUIT;       // easy exit even if C&C breaks down!

        for(i=0; i<numcommands; i++) {
                if(curcommands[i].key == key.c)
                        return curcommands[i].cmd;
        }

        dsprintf("unknown key: %d", key.c);

        return 0;
}

void init_commands()
{
        curcommands = outsidecommands;
        numcommands = (sizeof(outsidecommands) / sizeof(cmd_t));
}
