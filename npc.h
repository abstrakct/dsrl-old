/*
 * Dark Shadows - The Roguelike
 *
 * NPCs
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef _NPC_H
#define _NPC_H

void spawn_npcs(int num, void *p);
void process_npcs(level_t *l);
void move_npc(actor_t *m);

#endif