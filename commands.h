/*
 * Dark Shadows - The Roguelike
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef _COMMANDS_H
#define _COMMANDS_H

#include <libtcod/libtcod.h>

typedef struct {
        TCOD_key_t key;
        int cmd;
        char desc[50];
} cmd_t;

#define CMD_LEFT           1
#define CMD_RIGHT          2
#define CMD_UP             3
#define CMD_DOWN           4
#define CMD_QUIT           5
#define CMD_LONGLEFT       6
#define CMD_LONGRIGHT      7
#define CMD_LONGUP         8
#define CMD_LONGDOWN       9
#define CMD_TOGGLEFOV     10
#define CMD_SAVE          11
#define CMD_LOAD          12
#define CMD_ENTERDUNGEON  13
#define CMD_LEAVEDUNGEON  14
#define CMD_PICKUP        15
#define CMD_DROP          16
#define CMD_WIELDWEAR     17
#define CMD_IDENTIFYALL   18
#define CMD_SKILLSCREEN   19
#define CMD_NW            20
#define CMD_NE            21
#define CMD_SW            22
#define CMD_SE            23
#define CMD_INVENTORY     24
#define CMD_REST          25
#define CMD_DESCEND       26
#define CMD_ASCEND        27
#define CMD_UNWIELDWEAR   28
#define CMD_PATHFINDER    29
#define CMD_MOVE_ON       30

// development/debug commands
#define CMD_FLOODFILL     1001
#define CMD_SPAWNMONSTER  1002
#define CMD_WIZARDMODE    1003
#define CMD_DUMPOBJECTS   1004
#define CMD_INCFOV        1005
#define CMD_DECFOV        1006
#define CMD_DUMPCOLORS    1007
#define CMD_INCTIME       1008

// prototypes

int get_command();
void init_commands();

#endif
// vim: fdm=syntax guifont=Terminus\ 8
