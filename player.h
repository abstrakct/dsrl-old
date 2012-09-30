#ifndef _PLAYER_H
#define _PLAYER_H
/*
 * Dark Shadows - The Roguelike
 *
 * This file deals with general player stuff
 *
 * Copyright 2012 Rolf Klausen
 */

#define PC_VAMPIRE   1
#define PC_WEREWOLF  2
#define PC_PHOENIX   3
#define PC_WITCH     4
#define PC_WARLOCK   5
#define PC_HUMAN     6
#define PC_MANMADE   7
#define PC_GOVERNESS 8
#define PC_DOCTOR    9
#define PC_SERVANT   10
#define PC_GYPSY     11
#define PC_ZOMBIE    12
#define PC_PROFESSOR 13

#define PC_TYPEDEFS  12

typedef struct {
        char title[20];
        int  starthp;
        long flags;
} pc_typedef_t;


// Player character type flags
#define PCT_MUST_BE_MALE   (1 << 0)
#define PCT_MUST_BE_FEMALE (1 << 1)
#define PCT_IS_UNDEAD      (1 << 2)

// Function prototypes

void init_pc_types();


#endif
// vim: fdm=syntax guifont=Terminus\ 8
