/*
 * Dark Shadows - The Roguelike
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
#include "dsrl.h"

unsigned int mid_counter;

aifunction aitable[] = {
        simpleai,
        advancedai,
        hostile_ai
};


co get_next_step(void *actor)
{
        actor_t *a;
        co c;
        int x, y;

        a = (actor_t*)actor;
        
        TCOD_path_compute(a->path, a->x, a->y, a->goalx, a->goaly);
        if(!TCOD_path_walk(a->path, &x, &y, true)) {
                c.x = 0;
                c.y = 0;
        } else {
                c.x = x;
                c.y = y;
        }

        return c;
}

int simpleoutdoorpathfinder(actor_t *m)
{
        int choice;
        int oy, ox;
        //co c;

        oy = m->y;
        ox = m->x;

        if(m->y <= 2)
                return true;
        if(m->x <= 2)
                return true;

        if(!m->goalx || !m->goaly || m->x == m->goalx || m->y == m->goaly) {
                // basically, if we have no goal, or have reached the goal, set a new goal.
                m->goalx = ri(1, world->curlevel->xsize - 1);
                m->goaly = ri(1, world->curlevel->ysize - 1);
                while(!monster_passable(world->curlevel, m->goaly, m->goalx)) {
                        m->goalx = ri(1, world->curlevel->xsize - 1);
                        m->goaly = ri(1, world->curlevel->ysize - 1);
                }
        }

        choice = ri(1,100);
        if(choice <= 45) {
                if(m->x > m->goalx)
                        m->x--;
                if(m->x < m->goalx)
                        m->x++;
        } else if(choice > 45 && choice <= 90) {
                if(m->y > m->goaly)
                        m->y--;
                if(m->y < m->goaly)
                        m->y++;
        } else if(choice > 90) {
                // maybe not extremely useful, but adds randomness to the movements,
                // as if the creature's attention was briefly caught by something else..
                
                m->y += ri(-1, 1);
                m->x += ri(-1, 1);

         /*       switch(choice) {
                        case 91:
                                m->x--;
                                m->y++;
                                break;
                        case 92:
                                m->y++;
                                break;
                        case 93:
                                m->y++;
                                m->x++;
                                break;
                        case 94:
                                m->x--;
                                break;
                        case 95:
                                break;
                        case 96:
                                m->x++;
                                break;
                        case 97:
                                m->x--;
                                m->y--;
                                break;
                        case 98:
                                m->y--;
                                break;
                        case 99:
                                m->x++;
                                m->y--;
                                break;
                        case 100:
                                break;
                }*/
        }

        if(monster_passable(world->curlevel, m->y, m->x)) {
                world->cmap[oy][ox].monster = NULL;
                world->cmap[m->y][m->x].monster = m;
                return true;
        } else {
                m->y = oy;
                m->x = ox;
                return false;
        }

}

void simpleai(monster_t *m)
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
                attack(m, player);
        }

        if(monster_passable(world->curlevel, m->y, m->x)) {
                world->cmap[oy][ox].monster = NULL;
                world->cmap[m->y][m->x].monster = m;
        } else {
                m->x = ox; m->y = oy;
        }
}

void advancedai(monster_t *m)
{
        simpleai(m);
}

void newpathfinder_chaseplayer(actor_t *m)
{
        int dx, dy;

        dx = player->x - m->x;
        dy = player->y - m->y;

        dx = max(min(1, dx), -1);
        dy = max(min(1, dy), -1);

        m->y += dy;
        m->x += dx;
}

/*void oldhostile_ai(actor_t *m)
{
        int oy, ox;
        co c;

        oy = m->y;
        ox = m->x;

        if(m->attacker && next_to(m, m->attacker)) {
                attack(m, m->attacker);
                return;
        }

        if(next_to(m, player)) {
                m->attacker = player;
                attack(m, m->attacker);
                return;
        }

        if(actor_in_lineofsight(m, player)) {
                c = get_next_step(m->y, m->x);

                if(monster_passable(world->curlevel, m->y + c.y, m->x + c.x)) {
                        m->y += c.y;
                        m->x += c.x;
                        world->cmap[oy][ox].monster = NULL;
                        world->cmap[m->y][m->x].monster = m;
                }
        } else {
                m->attacker = NULL;
                while(!simpleoutdoorpathfinder(m));
        }
}*/

/**
 * @brief A simple, but effective AI function which will attack the player or other hostile creatures - or chase them if necessary!
 *
 * @param m The monster/actor which is performing this hostility.
 */
void hostile_ai(actor_t *m)
{
        int oy, ox;
        co c;

        oy = m->y;
        ox = m->x;

        if(m->attacker && next_to(m, m->attacker)) {
                attack(m, m->attacker);
                return;
        }

        if(next_to(m, player) && !is_invisible(player)) {
                m->attacker = player;
                attack(m, m->attacker);
                return;
        }

        if(actor_in_lineofsight(m, player)) {
                m->goalx = player->x;
                m->goaly = player->y;
        } else {
                m->attacker = NULL;
                do {
                        m->goalx = ri(1, world->curlevel->xsize-1);
                        m->goaly = ri(1, world->curlevel->ysize-1);
                } while(!monster_passable(world->curlevel, m->goaly, m->goalx));
        }

        c = get_next_step(m);

        if(c.x == 0 && c.y == 0) {
                return;
        } else {
                m->y = c.y;
                m->x = c.x;
                world->cmap[oy][ox].monster = NULL;
                world->cmap[m->y][m->x].monster = m;
        }
}

/**
 * @brief Callback function for libtcod pathfinding.
 *
 * @param xFrom Source X
 * @param yFrom Source Y
 * @param xTo   Dest. X
 * @param yTo   Dest. Y
 * @param user_data Pointer to the level where the pathfinding is taking place. 
 *
 * @return 1.0 if Dest X,Y is passable for a monster, 0.0 if not.
 */
float monster_path_callback_func(int xFrom, int yFrom, int xTo, int yTo, void *user_data)
{
        level_t *l;
        float f;

        l = (level_t*)user_data;
        if(monster_passable(l, yTo, xTo))
                f = 1.0f;
        else
                f = 0.0f;

        return f;
}

void heal_monster(actor_t *m, int num)
{
        increase_hp(m, num);
        dsprintf("  The %s looks a bit healthier! (%d)", m->name, num);
}

/**
 * @brief Make a move for a specific monster. Heal the monster if appropriate.
 *
 * @param m Monster to move.
 *
 */
void move_monster(monster_t *m)
{
        int i;

        if(!m) {
                fprintf(stderr, "DEBUG: %s:%d - no such monster!\n", __FILE__, __LINE__);
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

        if(!hasbit(m->flags, MF_SLEEPING)) {
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
        }

        schedule_monster(m);
}

/**
 * @brief See if there are any monsters in players LOS. If so, take note.
 */
void look_for_monsters()
{
        monster_t *m;

        //gtprintf("looking for monsters...");

        m = world->curlevel->monsters;
        if(!m)
                return;


        while(m) {
                m = m->next;
                while(m && hasbit(m->flags, MF_ISDEAD))
                        m = m->next;

                if(m) {
                        if(actor_in_lineofsight(player, m)) {
                                if(!hasbit(m->flags, MF_SEENBYPLAYER)) {
                                        setbit(m->flags, MF_SEENBYPLAYER);
                                        dsprintf("%s comes into view!", Upper(a_an(m->name)));
                                        schedule_monster(m);
                                }
                        }
                }
        }
}



monster_t get_monsterdef(int n)
{
        monster_t *tmp;

        tmp = monsterdefs->head->next;
        while(tmp->id != n) {
                tmp = tmp->next;
        }

        return *tmp;
}

/*
 * place a spawned monster at (y,x)
 */
bool place_monster_at(int y, int x, monster_t *monster, level_t *l)
{
        monster->x = x;
        monster->y = y;
        if(monster_passable(l, y, x) && l->c[monster->y][monster->x].monster == NULL) {
                l->c[monster->y][monster->x].monster = monster;
                monster->path = TCOD_path_new_using_function(l->xsize, l->ysize, monster_path_callback_func, l, 1.0f);
                return true;
        } else {
                return false;
        }
}

void spawn_monster(int n, monster_t *head, int maxlevel)
{
        monster_t *tmp;
        int hpadj;

        tmp = head->next;
        head->next = dsmalloc(sizeof(monster_t));
        *head->next = get_monsterdef(n);
        hpadj = head->next->level * 2;
        head->next->maxhp += ri((-(hpadj/2)), hpadj);
        head->next->hp = head->next->maxhp;

        head->next->next = tmp;
        head->next->prev = head;
        head->next->head = head;
        setbit(head->next->flags, MF_SLEEPING);
        //dsprintf("spawned monster %s\n", head->next->name);
        
        mid_counter++;
        head->next->mid = mid_counter;
}

void kill_monster(void *level, monster_t *m, actor_t *killer)
{
        level_t *l;
        l = (level_t *) level;

        if(l->c[m->y][m->x].monster == m) {
                // we probably should free/remove dead monsters, but something keeps going wrong, cheap cop-out:
                // also, this has it's advantages later (can be used for listing killed monsters).
                setbit(l->c[m->y][m->x].monster->flags, MF_ISDEAD);
                l->c[m->y][m->x].monster = NULL;
                if(killer == player) {
                        player->kills++;
                }
        } else {
                dsprintf("monster's x&y doesn't correspond to cell?");
        }
}

void unspawn_monster(monster_t *m)
{
        if(m) {
                m->prev->next = m->next;
                if(m->next)
                        m->next->prev = m->prev;
                dsfree(m);
        }
}

/*
 * spawn a monster and place it at (y,x)
 */
bool spawn_monster_at(int y, int x, int n, monster_t *head, void *level, int maxlevel)
{
        spawn_monster(n, head, maxlevel);
        if(head->next->level > maxlevel) {
                unspawn_monster(head->next);
                return false;
        }

        if(!place_monster_at(y, x, head->next, (level_t *) level)) {
                unspawn_monster(head->next);
                return false;
        }

        return true;
}

/*
 * Spawn num monsters of maximum level max_level, on level l
 * (yeah, level is used for two things, and confusing... i should change the terminology!
 */
void spawn_monsters(int num, int max_level, void *p)
{
        int i, x, y, m;
        level_t *l;

        i = 0;
        l = (level_t *) p;
        while(i < num) {
                x = 1; y = 1; m = ri(1, game->monsterdefs);
                while(!spawn_monster_at(y, x, m, l->monsters, l, max_level)) { 
                        x = ri(1, l->xsize-1);
                        y = ri(1, l->ysize-1);
                        m = ri(1, game->monsterdefs);
                }

                i++;
                game->num_monsters++;
        }
        //fprintf(stderr, "DEBUG: %s:%d - spawn_monsters spawned %d monsters (should spawn %d)\n", __FILE__, __LINE__, i, num);
}
// vim: fdm=syntax guifont=Terminus\ 8
