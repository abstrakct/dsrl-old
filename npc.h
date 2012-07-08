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
void spawn_named_npc(char *name, void *level);
void move_npc(actor_t *m);
void look_for_npcs();
void dump_npcs();

void generate_family(actor_t *man, actor_t *woman, enum fam family, int startyear);

#endif
