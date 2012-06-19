/*
 * Dark Shadows - The Roguelike
 *
 * NPC names
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef _NPC_NAMES_H
#define _NPC_NAMES_H

enum fam {
        evans = 0,
        loomis,
        stokes,
        bradford,
        blair,
        trask,
        stoddard,
        malloy, 
        mcguire, 
        patterson, 
        peterson, 
        devlin, 
        morgan, 
        drummond, 
        chavez,
        forbes, 
        woodard, 
        winters, 
        braithwaite, 
        hanley, 
        hackett, 
        gifford,
        garner, 
        faye, 
        dupres, 
        murdoch, 
        stockbridge,
        collins
};

void generate_npc_name(void *a, bool male, bool firstnameonly);

#endif
