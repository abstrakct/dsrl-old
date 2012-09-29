/*
 * Dark Shadows - The Roguelike
 *
 * This file deals with general player stuff
 *
 * Copyright 2012 Rolf Klausen
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include "npc-names.h"
#include "objects.h"
#include "actor.h"
#include "monsters.h"
#include "utils.h"
#include "world.h"
#include "datafiles.h"
#include "display.h"
#include "dstime.h"
#include "npc.h"
#include "player.h"
#include "dsrl.h"

char *pc_strings[] = {
        "",
        "vampire",
        "werewolf",
        "phoenix",
        "witch",
        "warlock",
        "human",
        "man-made monster",
        "governess",
        "doctor",
        "servant",
        "gypsy",
        "zombie"
};

pc_typedef_t pc_typedefs[PC_TYPEDEFS];

pc_typedef_t pct_human = {
        "human",
        10
};


void init_pc_types()
{
        pc_typedefs[PC_HUMAN] = pct_human;
}







// vim: fdm=syntax guifont=Terminus\ 8
