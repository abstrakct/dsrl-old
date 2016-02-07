#ifndef _PLAYER_H
#define _PLAYER_H
/*
 * Dark Shadows - The Roguelike
 *
 * This file deals with general player stuff
 *
 * Copyright 2012 Rolf Klausen
 */

enum {
        vampire,
        werewolf,
        phoenix,
        witch,
        warlock,
        human,
        manmade,
        governess,
        doctor,
        servant,
        gypsy,
        zombie,
        professor,
        ghost,
} role_e;

typedef struct {
        char title[20];
        int  starthp;
        long flags;
        bool mustbemale, mustbefemale;
} role_t;

#define ROLE_VAMPIRE   1
#define ROLE_WEREWOLF  2
#define ROLE_PHOENIX   3
#define ROLE_WITCH     4
#define ROLE_WARLOCK   5
#define ROLE_HUMAN     6
#define ROLE_MANMADE   7
#define ROLE_GOVERNESS 8
#define ROLE_DOCTOR    9
#define ROLE_SERVANT   10
#define ROLE_GYPSY     11
#define ROLE_ZOMBIE    12
#define ROLE_PROFESSOR 13

#define ROLE_TYPEDEFS  13


// Function prototypes

void init_roles();

#endif
// vim: fdm=syntax guifont=Terminus\ 8
