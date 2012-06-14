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

void npc_ai(actor_t *m)
{
        int dir, ox, oy;

        //dsprintf("hello it's simpleai!");
        dir = ri(1,9);
        ox = m->x; oy = m->y;

        switch(dir) {
                case 1:
                        m->x--;
                        m->y++;
                        break;
                case 2: 
                        m->y++;
                        break;
                case 3:
                        m->y++;
                        m->x++;
                        break;
                case 4:
                        m->x--;
                        break;
                case 5: 
                        break;
                case 6:
                        m->x++;
                        break;
                case 7:
                        m->x--;
                        m->y--;
                        break;
                case 8:
                        m->y--;
                        break;
                case 9:
                        m->x++;
                        m->y--;
                        break;
        }

        if(m->x == plx && m->y == ply) {
                m->x = ox; m->y = oy;
                //attack(m, player);
        }

        if(monster_passable(world->curlevel, m->y, m->x)) {
                world->cmap[oy][ox].npc = NULL;
                world->cmap[m->y][m->x].npc = m;
        } else {
                m->x = ox; m->y = oy;
        }
}

void process_npcs(level_t *l)
{
        actor_t *m;

        //dsprintf("processing npcs...");

        m = l->npcs;
        if(!m)
                return;


        while(m) {
                m = m->next;
                while(m && hasbit(m->flags, MF_ISDEAD))
                        m = m->next;

                if(m) {
                        //if(!hasbit(m->flags, MF_SEENBYPLAYER)) {
                        //        setbit(m->flags, MF_SEENBYPLAYER);
                        //        dsprintf("%s comes into view!", Upper((m->name)));
                                schedule_npc(m);
                        //}
                }
        }
}

/**
 * @brief Make a move for a specific npc. 
 *
 * @param m NPC to move.
 *
 */
void move_npc(actor_t *m)
{
        //int i;

        //dsprintf("moving NPC %s", m->name);

        if(!m) {
                fprintf(stderr, "DEBUG: %s:%d - no such npc!\n", __FILE__, __LINE__);
                return;
        }

        if(hasbit(m->flags, MF_ISDEAD)) {
                //fprintf(stderr, "DEBUG: %s:%d - monster is dead!\n", __FILE__, __LINE__);
                return;
        }

        if(hasbit(m->flags, MF_SLEEPING)) {
                if(actor_in_lineofsight(m, player))
                        clearbit(m->flags, MF_SLEEPING);
        }

        /*if(!hasbit(m->flags, MF_SLEEPING)) {
                hostile_ai(m);
                if(m->hp < m->maxhp) {
                        i = 17 - m->attr.phy;
                        if(i <= 0)
                                i = 1;
                        if(game->tick % i) {
                                if(perc(40+m->attr.phy)) {
                                        int j;

                                        j = ability_modifier(m->attr.phy);
                                        if(j < 1)
                                                j = 1;
                                        heal_monster(m, ri(1, j));
                                }
                        }
                }
        }*/

        npc_ai(m);
        schedule_npc(m);
}

/*
 * place a spawned npc at (y,x)
 */
bool place_npc_at(int y, int x, actor_t *npc, level_t *l)
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
        
        if(trueorfalse()) {
                generate_npc_name(head->next->name, true);
                setbit(head->next->flags, MF_MALE);
        } else {
                generate_npc_name(head->next->name, false);
                clearbit(head->next->flags, MF_MALE);
        }
        head->next->speed = 10;

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
