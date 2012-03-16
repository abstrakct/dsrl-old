/*
 * Dark Shadows - The Roguelike
 *
 * Various debugging stuff.
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef _DS_DEBUG_H
#define _DS_DEBUG_H

void dump_monsterdefs();
void dump_monsters(monster_t *list);
void dump_objects(inv_t *i);
void dump_action_queue();

#endif
