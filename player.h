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

#define PC_TYPEDEFS  12

typedef struct {
        char title[20];
        int  starthp;
} pc_typedef_t;




// Function prototypes

void init_pc_types();


#endif
// vim: fdm=syntax guifont=Terminus\ 8
