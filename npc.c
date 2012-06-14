/*
 * Dark Shadows - The Roguelike
 *
 * NPCs.
 *
 * Copyright 2011 Rolf Klausen
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "objects.h"
#include "actor.h"
#include "monsters.h"
#include "utils.h"
#include "world.h"
#include "datafiles.h"
#include "display.h"
#include "dstime.h"
#include "npc.h"
#include "npc-names.h"
#include "dsrl.h"

/*
 * place a spawned npc at (y,x)
 */
bool place_npc_at(int y, int x, monster_t *npc, level_t *l)
{
        npc->x = x;
        npc->y = y;
        if(monster_passable(l, y, x) && l->c[npc->y][npc->x].npc == NULL) {
                l->c[npc->y][npc->x].npc = npc;
                //npc->path = TCOD_path_new_using_function(l->xsize, l->ysize, npc_path_callback_func, l, 1.0f);
                return true;
        } else {
                return false;
        }
}

void spawn_npc(actor_t *head)
{
        actor_t *tmp;
        int hpadj;

        tmp = head->next;
        head->next = dsmalloc(sizeof(actor_t));
        
        generate_npc_name(head->next->name, trueorfalse());

        hpadj = head->next->level * 2;
        head->next->maxhp += ri((-(hpadj/2)), hpadj);
        head->next->hp = head->next->maxhp;

        head->next->next = tmp;
        head->next->prev = head;
        head->next->head = head;
        //setbit(head->next->flags, MF_SLEEPING);
        
        mid_counter++;
        head->next->mid = mid_counter;


fprintf(stderr, "DEBUG: %s:%d - Spawned NPC %s!\n", __FILE__, __LINE__, head->next->name);
}

void kill_npc(void *level, actor_t *m, actor_t *killer)
{
        level_t *l;
        l = (level_t *) level;

        if(l->c[m->y][m->x].npc == m) {
                // we probably should free/remove dead npcs, but something keeps going wrong, cheap cop-out:
                // also, this has it's advantages later (can be used for listing killed npcs).
                setbit(l->c[m->y][m->x].npc->flags, MF_ISDEAD);
                l->c[m->y][m->x].npc = NULL;
                if(killer == player) {
                        player->kills++;
                }
        } else {
                dsprintf("npc's x&y doesn't correspond to cell?");
        }
}

void unspawn_npc(actor_t *m)
{
        if(m) {
fprintf(stderr, "  DEBUG: %s:%d - Unspawning NPC %s!\n", __FILE__, __LINE__, m->name);
                m->prev->next = m->next;
                if(m->next)
                        m->next->prev = m->prev;
                dsfree(m);
        }
}

/*
 * spawn a npc and place it at (y,x)
 */
bool spawn_npc_at(int y, int x, actor_t *head, void *level)
{
        spawn_npc(head);
        /*if(head->next->level > maxlevel) {
                unspawn_npc(head->next);
                return false;
        }*/

        if(!place_npc_at(y, x, head->next, (level_t *) level)) {
                unspawn_npc(head->next);
                return false;
        }

        return true;
}

/**
 * @brief Spawn a number of NPCs
 *
 * @param num How many to spawn
 * @param p Pointer to the level on which to spawn 'em
 */
void spawn_npcs(int num, void *p)
{
        int i, x, y;
        level_t *l;

        i = 0;
        l = (level_t *) p;
        while(i < num) {
                x = 1; y = 1;
                while(!spawn_npc_at(y, x, l->npcs, l)) { 
                        x = ri(1, l->xsize-1);
                        y = ri(1, l->ysize-1);
                }

                i++;
                game->num_npcs++;
        }
        fprintf(stderr, "DEBUG: %s:%d - spawn_npcs spawned %d npcs (should spawn %d)\n", __FILE__, __LINE__, i, num);
}
// vim: fdm=syntax guifont=Terminus\ 8
