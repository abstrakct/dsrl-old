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

pc_typedef_t pc_typedefs[PC_TYPEDEFS];

pc_typedef_t pct_human     = {
        "human",
        10,
        0,
};
pc_typedef_t pct_vampire   = {
        "vampire",
        10,
        PCT_IS_UNDEAD,
};
pc_typedef_t pct_werewolf  = {
        "werewolf",
        10,
        PCT_IS_UNDEAD,
};
pc_typedef_t pct_phoenix   = {
        "phoenix",
        10,
        0,
};
pc_typedef_t pct_witch     = {
        "witch",
        10,
        PCT_MUST_BE_FEMALE,
};
pc_typedef_t pct_warlock   = {
        "warlock",
        10,
        PCT_MUST_BE_MALE,
};
pc_typedef_t pct_zombie    = {
        "zombie",
        10,
        PCT_IS_UNDEAD,
};
pc_typedef_t pct_gypsy     = {
        "gypsy",
        10,
        0,
};
pc_typedef_t pct_doctor    = {
        "doctor",
        10,
        0,
};
pc_typedef_t pct_governess = {
        "governess",
        10,
        PCT_MUST_BE_FEMALE,
};
pc_typedef_t pct_servant   = {
        "servant",
        10,
        0,
};
pc_typedef_t pct_manmade   = {
        "man-made monster",
        10,
        PCT_IS_UNDEAD,
};
pc_typedef_t pct_professor = {
        "professor",
        10,
        0,
};

void init_pc_types()
{
        pc_typedefs[PC_HUMAN]     = pct_human;
        pc_typedefs[PC_VAMPIRE]   = pct_vampire;
        pc_typedefs[PC_WEREWOLF]  = pct_werewolf;
        pc_typedefs[PC_PHOENIX]   = pct_phoenix;
        pc_typedefs[PC_WITCH]     = pct_witch;
        pc_typedefs[PC_WARLOCK]   = pct_warlock;
        pc_typedefs[PC_MANMADE]   = pct_manmade;
        pc_typedefs[PC_GOVERNESS] = pct_governess;
        pc_typedefs[PC_DOCTOR]    = pct_doctor;
        pc_typedefs[PC_SERVANT]   = pct_servant;
        pc_typedefs[PC_ZOMBIE]    = pct_zombie;
        pc_typedefs[PC_GYPSY]     = pct_gypsy;
        pc_typedefs[PC_PROFESSOR] = pct_professor;
}



// vim: fdm=syntax guifont=Terminus\ 8
