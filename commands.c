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

/*           keycode         char pressed lalt lctrl ralt rctrl shift */
cmd_t outsidecommands[] = {
        { { TCODK_DOWN,         0,   1,     0,   0,   0,    0,    0 }, CMD_DOWN,        "Move down" },
        { { TCODK_CHAR,       'j',   1,     0,   0,   0,    0,    0 }, CMD_DOWN,        "Move down" },
        { { TCODK_UP,           0,   1,     0,   0,   0,    0,    0 }, CMD_UP,          "Move up" },
        { { TCODK_CHAR,       'k',   1,     0,   0,   0,    0,    0 }, CMD_UP,          "Move up" },
        { { TCODK_LEFT,         0,   1,     0,   0,   0,    0,    0 }, CMD_LEFT,        "Move left" },
        { { TCODK_CHAR,       'h',   1,     0,   0,   0,    0,    0 }, CMD_LEFT,        "Move left" },
        { { TCODK_RIGHT,        0,   1,     0,   0,   0,    0,    0 }, CMD_RIGHT,       "Move right" },
        { { TCODK_CHAR,       'l',   1,     0,   0,   0,    0,    0 }, CMD_RIGHT,       "Move right" },
        { { TCODK_CHAR,       'y',   1,     0,   0,   0,    0,    0 }, CMD_NW,          "Move up-left" },
        { { TCODK_CHAR,       'u',   1,     0,   0,   0,    0,    0 }, CMD_NE,          "Move up-right" },
        { { TCODK_CHAR,       'b',   1,     0,   0,   0,    0,    0 }, CMD_SW,          "Move down-left" },
        { { TCODK_CHAR,       'n',   1,     0,   0,   0,    0,    0 }, CMD_SE,          "Move down-right" },
        { { TCODK_CHAR,       'w',   0,     0,   0,   0,    0,    0 }, CMD_WIELDWEAR,   "Wield or wear an item" },
        { { TCODK_CHAR,       'r',   0,     0,   0,   0,    0,    0 }, CMD_UNWIELDWEAR, "Remove or unwield an item" },
        { { TCODK_CHAR,       ',',   1,     0,   0,   0,    0,    0 }, CMD_PICKUP,      "Pick up something" },
        { { TCODK_CHAR,       '.',   1,     0,   0,   0,    0,    0 }, CMD_REST,        "Rest one turn" },
        //{ { TCODK_CHAR,       '<',   1,     0,   0,   0,    0,    0 }, CMD_ASCEND,      "Go up stairs" },
        //{ { TCODK_CHAR,       '>',   1,     0,   0,   0,    0,    0 }, CMD_DESCEND,     "Go down stairs" },
        { { TCODK_CHAR,       '<',   1,     0,   0,   0,    0,    0 }, CMD_USE_EXIT,    "Use exit" },
        { { TCODK_CHAR,       '>',   1,     0,   0,   0,    0,    0 }, CMD_USE_EXIT,    "Use exit" },
        { { TCODK_CHAR,       'd',   0,     0,   0,   0,    0,    0 }, CMD_DROP,        "Drop an object" },
        //{ { TCODK_CHAR,       'i', 1, 0, 0, 0, 0, 0 }, CMD_INVENTORY,   "Show inventory" },
        //{ TCODK_F5,  CMD_SAVE,        "Save" },
        //{ TCODK_F6,  CMD_LOAD,        "Load" },
#ifdef DEVELOPMENT_MODE
        { { TCODK_F1,           0, 1, 0, 0, 0, 0, 0 }, CMD_WIZARDMODE,  "Toggle wizard mode" },
        //{ TCODK_F2,  CMD_INCTIME,     "Time travel!?" },
        { { TCODK_CHAR,       '+', 1, 0, 0, 0, 0, 0 }, CMD_INCFOV,      "Increase FOV" },
        { { TCODK_CHAR,       '-', 1, 0, 0, 0, 0, 0 }, CMD_DECFOV,      "Decrease FOV" },
        { { TCODK_CHAR,       'f', 1, 0, 0, 0, 0, 0 }, CMD_FLOODFILL,   "Floodfill (debug)" },
        { { TCODK_CHAR,       's', 1, 0, 0, 0, 0, 0 }, CMD_SPAWNMONSTER,"Spawn monster" },
        { { TCODK_PAGEDOWN,    0,  1, 0, 0, 0, 0, 0 }, CMD_LONGDOWN,    "" },
        { { TCODK_PAGEUP,      0,  1, 0, 0, 0, 0, 0 }, CMD_LONGUP,      "" },
        { { TCODK_HOME,        0,  1, 0, 0, 0, 0, 0 }, CMD_LONGLEFT,    "" },
        { { TCODK_END,         0,  1, 0, 0, 0, 0, 0 }, CMD_LONGRIGHT,   "" },

        /*{ { TCODK_CHAR,       'K', 1, 0, 0, 0, 0, 0 }, CMD_LONGUP,      "" },
        { { TCODK_CHAR,       'J', 1, 0, 0, 0, 0, 0 }, CMD_LONGDOWN,    "" },
        { { TCODK_CHAR,       'H', 1, 0, 0, 0, 0, 0 }, CMD_LONGLEFT,    "" },
        { { TCODK_CHAR,       'L', 1, 0, 0, 0, 0, 0 }, CMD_LONGRIGHT,   "" },*/
        { { TCODK_CHAR,       'v', 1, 0, 0, 0, 0, 0 }, CMD_TOGGLEFOV,   "Toggle FOV" },
        //{ KEY_F(4),  CMD_DUMPOBJECTS, "Dump objects" },
        { { TCODK_CHAR,       'o', 1, 0, 0, 0, 0, 0 }, CMD_DUMPOBJECTS, "" },
        { { TCODK_CHAR,       'c', 1, 0, 0, 0, 0, 0 }, CMD_DUMPCOLORS, "" },
        { { TCODK_CHAR,       'p', 1, 0, 0, 0, 0, 0 }, CMD_PATHFINDER, "" },
#endif
        //{ , CMD_IDENTIFYALL, "Identify everything" },
        //{ , CMD_SKILLSCREEN, "Show skills" },
};

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
        bool b;

        key = dsgetch();
        if(key.vk == TCODK_NONE)
                return 0;

        if(key.vk == TCODK_ESCAPE) {
                b = yesno("Are you sure you want to quit?");
                if(b)
                        return CMD_QUIT;
        }

        for(i=0; i<numcommands; i++) {
                if(cmp_keystruct(curcommands[i].key, key))
                        return curcommands[i].cmd;
        }

        return 0;
}

void init_commands()
{
        curcommands = outsidecommands;
        numcommands = (sizeof(outsidecommands) / sizeof(cmd_t));
}
// vim: fdm=syntax guifont=Terminus\ 8
