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
        { { TCODK_DOWN,         0, 1, 0, 0, 0, 0, 0 }, CMD_DOWN,        "Move down" },
        { { TCODK_CHAR,       'j', 1, 0, 0, 0, 0, 0 }, CMD_DOWN,        "Move down" },
        { { TCODK_CHAR,       'k', 1, 0, 0, 0, 0, 0 }, CMD_UP,          "Move up" },
        { { TCODK_CHAR,       'h', 1, 0, 0, 0, 0, 0 }, CMD_LEFT,        "Move left" },
        { { TCODK_CHAR,       'l', 1, 0, 0, 0, 0, 0 }, CMD_RIGHT,       "Move right" },
        { { TCODK_CHAR,       'y', 1, 0, 0, 0, 0, 0 }, CMD_NW,          "Move up-left" },
        { { TCODK_CHAR,       'u', 1, 0, 0, 0, 0, 0 }, CMD_NE,          "Move up-right" },
        { { TCODK_CHAR,       'b', 1, 0, 0, 0, 0, 0 }, CMD_SW,          "Move down-left" },
        { { TCODK_CHAR,       'n', 1, 0, 0, 0, 0, 0 }, CMD_SE,          "Move down-right" },
        { { TCODK_CHAR,       'i', 1, 0, 0, 0, 0, 0 }, CMD_INVENTORY,   "Show inventory" },
        { { TCODK_CHAR,       'w', 1, 0, 0, 0, 0, 0 }, CMD_WIELDWEAR,   "Wield or wear an item" },
        { { TCODK_CHAR,       'r', 1, 0, 0, 0, 0, 0 }, CMD_UNWIELDWEAR, "Remove or unwield an item" },
        //{ TCODK_F5,  CMD_SAVE,        "Save" },
        //{ TCODK_F6,  CMD_LOAD,        "Load" },
        { { TCODK_CHAR,       ',', 1, 0, 0, 0, 0, 0 }, CMD_PICKUP,      "Pick up something" },
        { { TCODK_CHAR,       '.', 1, 0, 0, 0, 0, 0 }, CMD_REST,        "Rest one turn" },
        { { TCODK_CHAR,       '<', 1, 0, 0, 0, 0, 0 }, CMD_ASCEND,      "Go up stairs" },
        { { TCODK_CHAR,       '>', 1, 0, 0, 0, 0, 0 }, CMD_DESCEND,     "Go down stairs" },
        { { TCODK_CHAR,       'd', 1, 0, 0, 0, 0, 0 }, CMD_DROP,        "Drop an object" },
#ifdef DEVELOPMENT_MODE
        { { TCODK_CHAR,       'W', 1, 0, 0, 0, 0, 0 }, CMD_WIZARDMODE,  "Toggle wizard mode" },
        //{ TCODK_F2,  CMD_INCTIME,     "Time travel!?" },
        { { TCODK_CHAR,       '+', 1, 0, 0, 0, 0, 0 }, CMD_INCFOV,      "Increase FOV" },
        { { TCODK_CHAR,       '-', 1, 0, 0, 0, 0, 0 }, CMD_DECFOV,      "Decrease FOV" },
        { { TCODK_CHAR,       'f', 1, 0, 0, 0, 0, 0 }, CMD_FLOODFILL,   "Floodfill (debug)" },
        { { TCODK_CHAR,       's', 1, 0, 0, 0, 0, 0 }, CMD_SPAWNMONSTER,"Spawn monster" },
        //{ TCODK_PAGEDOWN, CMD_LONGDOWN,    "" },
        //{ TCODK_PAGEUP, CMD_LONGUP,      "" },
        { { TCODK_CHAR,       'K', 1, 0, 0, 0, 0, 0 }, CMD_LONGUP,      "" },
        { { TCODK_CHAR,       'J', 1, 0, 0, 0, 0, 0 }, CMD_LONGDOWN,    "" },
        { { TCODK_CHAR,       'H', 1, 0, 0, 0, 0, 0 }, CMD_LONGLEFT,    "" },
        { { TCODK_CHAR,       'L', 1, 0, 0, 0, 0, 0 }, CMD_LONGRIGHT,   "" },
        { { TCODK_CHAR,       'v', 1, 0, 0, 0, 0, 0 }, CMD_TOGGLEFOV,   "Toggle FOV" },
        //{ KEY_F(4),  CMD_DUMPOBJECTS, "Dump objects" },
        { { TCODK_CHAR,       'o', 1, 0, 0, 0, 0, 0 }, CMD_DUMPOBJECTS, "" },
        { { TCODK_CHAR,       'c', 1, 0, 0, 0, 0, 0 }, CMD_DUMPCOLORS, "" },
        { { TCODK_CHAR,       'p', 1, 0, 0, 0, 0, 0 }, CMD_PATHFINDER, "" },
#endif
        //{ , CMD_IDENTIFYALL, "Identify everything" },
        //{ , CMD_SKILLSCREEN, "Show skills" },
};

#endif

int cmp_keystruct(TCOD_key_t a, TCOD_key_t b)
{
        if((a.vk == b.vk) &&
           (a.pressed == b.pressed) &&
           (a.c == b.c) &&
           (a.lalt == b.lalt) &&
           (a.lctrl == b.lctrl) &&
           (a.ralt == b.ralt) &&
           (a.rctrl == b.rctrl) &&
           (a.shift == b.shift))
                return 1;                              /* they're the same */
        else
                return 0;
}

int get_command()
{
        int i;
        TCOD_key_t key;

        //TCOD_console_flush();

        key = TCOD_console_wait_for_keypress(false);

        if(key.vk == TCODK_ESCAPE)
                return CMD_QUIT;       // easy exit even if C&C breaks down!

        for(i=0; i<numcommands; i++) {
                if(cmp_keystruct(curcommands[i].key, key))
                        return curcommands[i].cmd;
        }

        //dsprintf("unknown key: %d", key.c);

        return 0;
}

void init_commands()
{
        curcommands = outsidecommands;
        numcommands = (sizeof(outsidecommands) / sizeof(cmd_t));
}
