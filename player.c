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

role_t roledefs[] = {
        { "vampire", 10, ACTOR_IS_UNDEAD, false, false },
        { "werewolf", 10, 0, false, false },
        { "phoenix", 10, 0, false, false },
        { "witch", 10, 0, false, true },
        { "warlock", 10, 0, true, false },
        { "human", 10, 0, false, false },
        { "man-made monster", 10, 0, false, false },
        { "governess", 10, 0, false, true },
        { "doctor", 10, 0, false, false },
        { "servant", 10, 0, false, false },
        { "gypsy", 10, 0, false, false },
        { "zombie", 10, ACTOR_IS_UNDEAD, false, false },
        { "professor", 10, 0, false, false },
        { "ghost", 10, ACTOR_IS_UNDEAD, false, false },
};


void init_roles()
{
}



// vim: fdm=syntax 
