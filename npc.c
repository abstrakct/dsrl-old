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
#include "npc-names.h"
#include "dsrl.h"

extern char *familyname[];

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

#define MALE 1
#define FEMALE 2
actor_t *spawn_npc(actor_t *head, int gender, bool firstnameonly)
{
        actor_t *tmp;
        int hpadj;

        tmp = head->next;
        head->next = dsmalloc(sizeof(actor_t));
        
        if(!gender) {     // then random
                if(trueorfalse()) {
                        generate_npc_name(head->next, true, firstnameonly);
                        setbit(head->next->flags, MF_MALE);
                } else {
                        generate_npc_name(head->next, false, firstnameonly);
                        clearbit(head->next->flags, MF_MALE);
                }
        }

        if(gender == MALE) {
                generate_npc_name(head->next, true, firstnameonly);
                setbit(head->next->flags, MF_MALE);
        }

        if(gender == FEMALE) {
                generate_npc_name(head->next, false, firstnameonly);
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
        
        return head->next;
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
//fprintf(stderr, "  DEBUG: %s:%d - Unspawning NPC %s!\n", __FILE__, __LINE__, m->name);
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
        spawn_npc(head, 0, false);
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
                fprintf(stderr, "DEBUG: %s:%d - Spawned NPC %s!\n", __FILE__, __LINE__, l->npcs->next->name);
        }
        fprintf(stderr, "DEBUG: %s:%d - spawn_npcs spawned %d npcs (should spawn %d)\n", __FILE__, __LINE__, i, num);
}

void spawn_named_npc(char *name, void *level)
{
        int x, y;
        level_t *l;

        l = (level_t *) level;
        x = 1; y = 1;
        while(!spawn_npc_at(y, x, l->npcs, l)) { 
                x = ri(1, l->xsize-1);
                y = ri(1, l->ysize-1);
        }
        strcpy(l->npcs->next->name, name);
        game->num_npcs++;

        fprintf(stderr, "DEBUG: %s:%d - Spawned named NPC %s!\n", __FILE__, __LINE__, l->npcs->next->name);
}

bool set_child(actor_t *child, actor_t *father, actor_t *mother)
{
        bool result;
        
        result = false;

        if(father->children < 6 && mother->children < 6) {
                father->child[father->children] = child;
                mother->child[mother->children] = child;
                father->children++;
                mother->children++;
                result = true;
        }

        return result;
} 

actor_t *add_child(actor_t *father, actor_t *mother)
{
        actor_t *child;
        bool c;

        if(trueorfalse())
                child = spawn_npc(world->npcs, MALE, true);
        else
                child = spawn_npc(world->npcs, FEMALE, true);

        strcat(child->name, " ");
        strcat(child->name, familyname[(int)father->family]); 
        child->family = father->family;

        child->father = father;
        child->mother = mother;
        c = set_child(child, father, mother);

        if(c)
                printf("add_child: %s and %s has given birth to %s!\n", father->name, mother->name, child->name);

        return child;
}



/**
 * @brief Generate/simulate a family!
 *
 * @param man    The "first man"
 * @param woman  The "first woman" (his wife)
 * @param family Which family?
 */
void generate_family(actor_t *man, actor_t *woman, enum fam family, int startyear)
{
        int year, lastyear, i;
        actor_t *child;

        if((!man && !woman)) {
                world->npcs = dsmalloc(sizeof(actor_t));

                man = spawn_npc(world->npcs, MALE, true);
                strcat(man->name, " ");
                strcat(man->name, familyname[(int)family]); 
                man->family = family;
                man->birth = startyear + ri(-5, 5);
                man->death = man->birth + d(25, 3);

                woman = spawn_npc(world->npcs, FEMALE, true);
                strcat(woman->name, " ");
                strcat(woman->name, familyname[(int)family]); 
                woman->family = family;
                woman->birth = startyear + ri(-5, 5);
                woman->death = woman->birth + d(25, 3);

                printf("Spawned ancestors!\n");
                printf("%s - %d-%d\n", man->name, man->birth, man->death);
                printf("%s - %d-%d\n", woman->name, woman->birth, woman->death);
        }

        if(!man && woman) {
                man = spawn_npc(world->npcs, MALE, true);
                strcat(man->name, " ");
                strcat(man->name, familyname[(int)family]); 
                man->family = family;
                man->birth = startyear;
                man->death = man->birth + d(25, 3);
        }

        if(man && !woman) {
                woman = spawn_npc(world->npcs, FEMALE, true);
                strcat(woman->name, " ");
                strcat(woman->name, familyname[(int)family]); 
                woman->family = family;
                woman->birth = startyear;
                woman->death = woman->birth + d(25, 3);
        }

        if(man->birth < woman->birth)
                year = man->birth;
        else
                year = woman->birth;

        year += ri(20, 22);
        lastyear = man->death;

        for(i = year; i <= lastyear; i++) {
                if(i == man->death)
                        setbit(man->flags, MF_ISDEAD);
                if(i == woman->death)
                        setbit(woman->flags, MF_ISDEAD);

                printf("Year: %d\n", i);
                if(perc(20)) {
                        child = add_child(man, woman);
                        if(child) {
                                if(hasbit(child->flags, MF_MALE)) 
                                        generate_family(child, 0, child->family, i);
                                else
                                        generate_family(0, child, child->family, i);
                        } else 
                                printf("???\n");
                }

                if(i == game->t.year)
                        break;

        }
}

// vim: fdm=syntax guifont=Terminus\ 8
