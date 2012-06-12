/*
 * Dark Shadows - The Roguelike
 * 
 * Copyright 2011 Rolf Klausen
 */

#define _XOPEN_SOURCE_EXTENDED

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <locale.h>
#include <time.h>
#include <signal.h>
#include <libconfig.h>
#include <getopt.h>

#include "objects.h"
#include "actor.h"
#include "monsters.h"
#include "utils.h"
#include "world.h"
#include "datafiles.h"
#include "display.h"
#include "debug.h"
#include "saveload.h"
#include "commands.h"
#include "dstime.h"
#include "dsrl.h"

char *otypestrings[50] = {
        "",
        "Gold",
        "Weapon",
        "Armor",
        "Bracelet",
        "Amulet",
        "Card",
        "Wand",
        "Potion",
        "",
        "",
        "",
        "",
        ""
};

// Important global variables
monster_t   *monsterdefs;
obj_t       *objdefs;
game_t      *game;
world_t     *world;
actor_t     *player;
ds_config_t dsconfig;
long        actionnum;
FILE        *messagefile;
bool        mapchanged;
bool        animate_only;
int         tempxsize, tempysize;
bool        loadgame;
void        *actiondata;
actor_t     *a_attacker, *a_victim;
struct      actionqueue *aq;
action_t    *act;

actor_t *a_attacker, *a_victim;

// Messages
message_t messages[500];
int currmess, maxmess;

/**
 * @brief Command line options
 */
struct option ds_options[] = {
        { "seed",    1,   0, 's' },
        { "load",    1,   0, 'l' },
        { "version", 0,   0, 'v' },
        { NULL,      0, NULL, 0  }
};

/**
 * @brief Initialize important variables and data structures.
 */
void init_variables()
{
        int i, j;

        garbageindex = 0;

        monsterdefs = (monster_t *) dsmalloc(sizeof(monster_t));
        monsterdefs->head = monsterdefs;
        mid_counter = 1000;

        objdefs = (obj_t *) dsmalloc(sizeof(obj_t));
        objdefs->head = objdefs;

        act = (action_t *) dsmalloc(sizeof(action_t) * MAXACT);
        for(i=0;i<MAXACT;i++)
                act[i].action = ACTION_FREESLOT;

        world = (world_t *) dsmalloc(sizeof(world_t));

        world->area = dscalloc(10, sizeof(level_t));    // allocate n levels, 0 = outside, 1..n = areas
        world->out = world->area;                      // i.e. it points to world->area[0]
        world->out->xsize = XSIZE;
        world->out->ysize = YSIZE;

        game = (game_t *) dsmalloc(sizeof(game_t));
        game->dead = 0;
        
        game->seed = time(0);
        srand(game->seed);

        game->createdareas = 0;
        generate_savefilename(game->savefile);
        loadgame = false;

        game->wizardmode = false;
        player = (actor_t *) dsmalloc(sizeof(actor_t));

        game->t.year   = 1967;
        game->t.month  = 4;
        game->t.day    = 18;
        game->t.hour   = 18;
        game->t.minute = 0;
        game->t.second = 0;
        i = ri(72, 18000);
        for(j=0;j<i;j++)
                inc_second(&game->t, 1);
}

/*! \brief Initialize player struct */
void init_player()
{
        // TODO: Character generation!!
        //plx = game->mapw / 2;
        //ply = game->maph / 2;
        ppx = plx - game->map.w / 2;
        ppy = ply - game->map.h / 2;
        game->mapcx = game->map.w - 2;
        game->mapcy = game->map.h - 2;
        player->viewradius = 42;
        player->level = 1;

        player->attr.str  = dice(3, 6, 0);
        player->attr.dex  = dice(3, 6, 0);
        player->attr.phy  = dice(3, 6, 0);
        player->attr.wis  = dice(3, 6, 0);
        player->attr.cha  = dice(3, 6, 0);
        player->attr.intl = dice(3, 6, 0);

        player->speed = 9;

        // TODO: Starting HP - FIX according to race etc.
        player->hp = player->maxhp = (dice(1, 8, 5)) + ability_modifier(player->attr.phy);

        player->path = TCOD_path_new_using_map(world->curlevel->map, 1.41f);

        strcpy(player->name, "Barnabas Collins");
}

/*! \brief Shutdown DS
 * Frees all allocated memory and closes open files
 */
void shutdown_ds()
{
        int i;

        i = garbageindex;
        while(i >= 0) {
               i--;
               if(garbage[i])
                       free((void *)garbage[i]);
        }
        
        if(messagefile)
                fclose(messagefile);
}

/*! \brief Parse the commandline
 * Parse the commandline
 * Most of this function is stolen from getopt's wikipedia page
 */
void parse_commandline(int argc, char **argv)
{
        int option_index = 0;
        int c;
        char s[15];

        while((c = getopt_long(argc, argv, "s:v", ds_options, &option_index)) != -1) {
                switch(c) {
                        case 's': game->seed = atoi(optarg);
                                  srand(game->seed);
                                  generate_savefilename(game->savefile);
                                  fprintf(stderr, "DEBUG: %s:%d - set random seed to %d (parse_commandline)\n", __FILE__, __LINE__, game->seed);
                                  break;
                        case 'v': get_version_string(s); printf("%s v%s\n", GAME_NAME, s); exit(0); break;
                        case 'l': strcpy(game->savefile, optarg);
                                  loadgame = true;
                                  break;
                        default:  printf("Unknown command line option 0%o -- ignoring!\n", c);
                }
        }
}

/*! \brief Fix the view */
void fixview()
{
        ppx = plx - (game->map.w / 2);
        ppy = ply - (game->map.h / 2);

        if(plx < 0)
                plx = 0;
        if(plx <= (ppx+(game->mapcx/6)))
                ppx--;
        if(plx >= (ppx+(game->mapcx/6*5)))
                ppx++;
        if(ppx >= world->curlevel->xsize - game->mapcx)
                ppx = world->curlevel->xsize - game->mapcx - 1;
        if(ppx < 0)
                ppx = 0;

        if(ply < 0)
                ply = 0;
        if(ply <= (ppy + (game->mapcy/6)))
                ppy--;
        if(ply >= (ppy + (game->mapcy/6*5)))
                ppy++;
        if(ppy >= world->curlevel->ysize - game->mapcy)
                ppy = world->curlevel->ysize - game->mapcy - 1;
        if(ppy < 0)
                ppy = 0;
}

/*! \brief Open a door
 *  \param x The X coordinate of the door.
 *  \param y The Y coordinate of the door.
 */
void open_door(int y, int x)
{
        clearbit(cf(y, x), CF_HAS_DOOR_CLOSED);
        setbit(cf(y, x), CF_HAS_DOOR_OPEN);
        TCOD_map_set_properties(world->curlevel->map, x, y, true, true);

        if(hasbit(cf(y+1,x), CF_HAS_DOOR_CLOSED))
                open_door(y+1,x);
        if(hasbit(cf(y-1,x), CF_HAS_DOOR_CLOSED))
                open_door(y-1,x);
        if(hasbit(cf(y,x+1), CF_HAS_DOOR_CLOSED))
                open_door(y,x+1);
        if(hasbit(cf(y,x-1), CF_HAS_DOOR_CLOSED))
                open_door(y,x-1);
}

/*! \brief Clear the actionqueue */
void clear_aq()
{
        struct actionqueue *tmp;

        while(aq->num) {
                tmp = aq->next;
                aq->next = tmp->next;
                dsfree(tmp);
                aq->num--;
        }
}

/*! \brief Setup attack - that is, do what's needed to perform an attack by the player.
 */
void setup_attack()
{
        int i;


        schedule_action(ACTION_ATTACK, player);

        i = d(1, 10);
        inc_second(&game->t, i);
        inc_second(&game->total, i);
}

void do_one_action(int action)
{
        action_t a;

        a.action = action;
        a.tick = game->tick;

        do_action(&a);
}

/**
 * @brief Do an action.
 *
 */
bool do_action(action_t *aqe)
{
        int oldy, oldx;
        int tmpy, tmpx;
        bool fullturn;
        obj_t *o;
        int i;
        exit_t *src, *dest;

        oldy = ply; oldx = plx;
        fullturn = true;

        switch(aqe->action) {
                case ACTION_PLAYER_MOVE_DOWN:
                        if(passable(world->curlevel, ply+1, plx)) {
                                if(world->curlevel->c[ply+1][plx].monster) {
                                        a_attacker = player;
                                        a_victim = world->curlevel->c[ply+1][plx].monster;
                                        setup_attack();
                                        break;
                                } else
                                        ply++;
                        } else {
                                fullturn = false;
                                break;
                        }

                        if(ply >= (ppy + (game->mapcy/6*5))) {
                                mapchanged = true;
                                ppy++;
                        }
                        if(ppy >= world->curlevel->ysize-game->mapcy) {
                                mapchanged = true;
                                ppy = world->curlevel->ysize - game->mapcy - 1;
                        }
                        if(ppy < 0)
                                ppy = 0;
                        break;
                case ACTION_PLAYER_MOVE_UP:
                        if(passable(world->curlevel, ply-1,plx)) {
                                if(world->curlevel->c[ply-1][plx].monster) {
                                        a_attacker = player;
                                        a_victim = world->curlevel->c[ply-1][plx].monster;
                                        setup_attack();
                                        break;
                                } else
                                        ply--;
                        } else {
                                fullturn = false;
                                break;
                        }

                        if(ply < 0)
                                ply = 0;
                        if(ply <= (ppy + (game->mapcy/6))) {
                                mapchanged = true;
                                ppy--;
                        }
                        if(ppy < 0)
                                ppy = 0;
                        break;
                case ACTION_PLAYER_MOVE_LEFT:
                        if(passable(world->curlevel, ply, plx-1)) {
                                if(world->curlevel->c[ply][plx-1].monster) {
                                        a_attacker = player;
                                        a_victim = world->curlevel->c[ply][plx-1].monster;
                                        setup_attack();
                                        break;
                                } else
                                        plx--;
                        } else {
                                fullturn = false;
                                break;
                        }

                        if(plx < 0)
                                plx = 0;
                        if(plx <= (ppx+(game->mapcx/6))) {
                                mapchanged = true;
                                ppx--;
                        }
                        if(ppx < 0)
                                ppx = 0;
                        break;
                case ACTION_PLAYER_MOVE_RIGHT:
                        if(passable(world->curlevel, ply,plx+1)) {
                                if(world->curlevel->c[ply][plx+1].monster) {
                                        a_attacker = player;
                                        a_victim = world->curlevel->c[ply][plx+1].monster;
                                        setup_attack();
                                        break;
                                } else
                                        plx++;
                        } else {
                                fullturn = false;
                                break;
                        }

                        if(plx >= (ppx+(game->mapcx/6*5))) {
                                mapchanged = true;
                                ppx++;
                        }
                        if(ppx >= world->curlevel->xsize-game->mapcx) {
                                mapchanged = true;
                                ppx = world->curlevel->xsize-game->mapcx-1;
                        }
                        if(ppx < 0)
                                ppx = 0;
                        break;
                case ACTION_PLAYER_MOVE_NW:
                        if(passable(world->curlevel, ply-1,plx-1)) {
                                if(world->curlevel->c[ply-1][plx-1].monster) {
                                        a_attacker = player;
                                        a_victim = world->curlevel->c[ply-1][plx-1].monster;
                                        setup_attack();
                                        break;
                                } else {
                                        ply--;
                                        plx--;
                                }
                        } else {
                                fullturn = false;
                                break;
                        }

                        if(ply < 0)
                                ply = 0;
                        if(ply <= (ppy + (game->mapcy/6))) {
                                mapchanged = true;
                                ppy--;
                        }
                        if(ppy < 0)
                                ppy = 0;

                        if(plx < 0)
                                plx = 0;
                        if(plx <= (ppx + (game->mapcx/6))) {
                                mapchanged = true;
                                ppx--;
                        }
                        if(ppx < 0)
                                ppx = 0;
                        break;
                case ACTION_PLAYER_MOVE_NE:
                        if(passable(world->curlevel, ply-1,plx+1)) {
                                if(world->curlevel->c[ply-1][plx+1].monster) {
                                        a_attacker = player;
                                        a_victim = world->curlevel->c[ply-1][plx+1].monster;
                                        setup_attack();
                                        break;
                                } else {
                                        ply--; plx++;
                                }
                        } else {
                                fullturn = false;
                                break;
                        }

                        if(plx >= world->curlevel->xsize)
                                plx = world->curlevel->xsize-1;
                        if(plx >= (ppx+(game->mapcx/6*5))) {
                                mapchanged = true;
                                ppx++;
                        }
                        if(ppx >= world->curlevel->xsize-game->mapcx) {
                                mapchanged = true;
                                ppx = world->curlevel->xsize-game->mapcx-1;
                        }
                        if(ppx < 0)
                                ppx = 0;

                        if(ply < 0)
                                ply = 0;
                        if(ply <= (ppy+(game->mapcy/6))) {
                                mapchanged = true;
                                ppy--;
                        }
                        if(ppy < 0)
                                ppy = 0;
                        break;
                case ACTION_PLAYER_MOVE_SW:
                        if(passable(world->curlevel, ply+1, plx-1)) {
                                if(world->curlevel->c[ply+1][plx-1].monster) {
                                        a_attacker = player;
                                        a_victim = world->curlevel->c[ply+1][plx-1].monster;
                                        setup_attack();
                                        break;
                                } else {
                                        ply++; plx--;
                                }
                        } else {
                                fullturn = false;
                                break;
                        }

                        if(ply >= world->curlevel->ysize)
                                ply = world->curlevel->ysize-1;
                        if(ply >= (ppy+(game->mapcy/6*5))) {
                                mapchanged = true;
                                ppy++;
                        }
                        if(ppy >= world->curlevel->ysize-game->mapcy) {
                                mapchanged = true;
                                ppy = world->curlevel->ysize-game->mapcy-1;
                        }
                        if(ppy < 0)
                                ppy = 0;

                        if(plx <= (ppx+(game->mapcx/6))) {
                                mapchanged = true;
                                ppx--;
                        }
                        if(ppx < 0)
                                ppx = 0;
                        break;
                case ACTION_PLAYER_MOVE_SE:
                        if(passable(world->curlevel, ply+1, plx+1)) {
                                if(world->curlevel->c[ply+1][plx+1].monster) {
                                        a_attacker = player;
                                        a_victim = world->curlevel->c[ply+1][plx+1].monster;
                                        setup_attack();
                                        break;
                                } else {
                                        ply++; plx++;
                                }
                        } else {
                                fullturn = false;
                                break;
                        }

                        if(ply >= world->curlevel->ysize)
                                ply = world->curlevel->ysize-1;
                        if(ply >= (ppy+(game->mapcy/6*5))) {
                                mapchanged = true;
                                ppy++;
                        }
                        if(ppy >= world->curlevel->ysize - game->mapcy) {
                                mapchanged = true;
                                ppy = world->curlevel->ysize - game->mapcy - 1;
                        }
                        if(ppy < 0)
                                ppy = 0;

                        if(plx >= world->curlevel->xsize)
                                plx = world->curlevel->xsize - 1;
                        if(plx >= (ppx+(game->mapcx/6*5))) {
                                mapchanged = true;
                                ppx++;
                        }
                        if(ppx >= world->curlevel->xsize - game->mapcx) {
                                mapchanged = true;
                                ppx = world->curlevel->xsize - game->mapcx-1;
                        }
                        if(ppx < 0)
                                ppx = 0;
                        break;
                case ACTION_PICKUP:
                        if(ci(ply, plx) && ci(ply, plx)->gold > 0) {
                                player->inventory->gold += ci(ply, plx)->gold;
                                ci(ply, plx)->gold -= ci(ply, plx)->gold;
                                youc(COLOR_INFO, "now have %d gold pieces.", player->inventory->gold);
                        }

                        if(ci(ply, plx) && ci(ply, plx)->num_used > 0) {
                                int slot;
                                slot = get_first_used_slot(ci(ply, plx));
                                pick_up(ci(ply, plx)->object[slot], player);
                                ci(ply, plx)->object[slot] = NULL;
                                ci(ply, plx)->num_used--;
                        }
                        break;
                case ACTION_ATTACK:
                        attack(a_attacker, a_victim);
                        break;
                case ACTION_MOVE_MONSTERS:
                        break;
                case ACTION_GO_DOWN_STAIRS:
                        if(game->currentlevel < game->createdareas) {
                                tmpy = world->curlevel->c[ply][plx].desty;
                                tmpx = world->curlevel->c[ply][plx].destx;
                                if(game->currentlevel == 0) {
                                        world->area[1].c[tmpy][tmpx].desty = ply;
                                        world->area[1].c[tmpy][tmpx].destx = plx;
                                        dsprintf("setting return destination to %d,%d", ply, plx);
                                }

                                game->currentlevel++;
                                world->cmap = world->area[game->currentlevel].c;
                                world->curlevel = &(world->area[game->currentlevel]);
                                if(game->currentlevel > 0)
                                        game->context = CONTEXT_INSIDE;

                                ply = tmpy;
                                plx = tmpx;

                                if(world->curlevel->type == 1)
                                        player->viewradius = 16;
                                else
                                        player->viewradius = 8;
                                player->path = TCOD_path_new_using_map(world->curlevel->map, 1.0f);
                        }
                        break;
                case ACTION_GO_UP_STAIRS:
                        tmpy = ply; tmpx = plx;
                        ply = world->curlevel->c[tmpy][tmpx].desty;
                        plx = world->curlevel->c[tmpy][tmpx].destx;

                        if(game->currentlevel > 0)
                                game->currentlevel--;
                        world->cmap = world->area[game->currentlevel].c;
                        world->curlevel = &(world->area[game->currentlevel]);

                        if(game->currentlevel == 0) {
                                game->context = CONTEXT_OUTSIDE;
                                player->viewradius = 45;
                        }
                        player->path = TCOD_path_new_using_map(world->curlevel->map, 1.0f);
                        break;
                case ACTION_USE_EXIT:
                        tmpy = ply; tmpx = plx;
                        src  = &world->curlevel->exit[world->curlevel->c[tmpy][tmpx].exitindex];
                        if(src->dest == -1) {
                                dsprintf("sorry! not currently available!");
                                break;
                        }
                        dest = &world->area[src->location].exit[src->dest];
                        //dsprintf("src: location=%d x=%d y=%d type=%d dest=%d", src->location, src->x, src->y, src->type, src->dest);
                        //dsprintf("dest: location=%d x=%d y=%d type=%d dest=%d", dest->location, dest->x, dest->y, dest->type, dest->dest);

                        ply = dest->y;
                        plx = dest->x;
                        world->curlevel = &world->area[src->location];
                        world->cmap     = world->area[src->location].c;
                        game->currentlevel = src->location;
                        newfov_initmap(&world->area[src->location]);
                        player->path = TCOD_path_new_using_map(world->curlevel->map, 1.0f);
                        update_screen();

                        //ply = world->curlevel->exit[world->curlevel->c[tmpy][tmpx].exitindex].
                        break;
                case ACTION_WIELDWEAR:
                        o = (obj_t *) actiondata;
                        if(o)
                                wieldwear(o);
                        else
                                dsprintf("That doesn't seem to work.");
                        player->ticks -= TICKS_WIELDWEAR;
                        break;
                case ACTION_UNWIELDWEAR:
                        o = (obj_t *) actiondata;
                        if(o)
                                unwieldwear(o);
                        else
                                dsprintf("That doesn't seem to work.");

                        break;
                case ACTION_DROP:
                        o = (obj_t *) actiondata;
                        if(o)
                                drop(o, player);
                        else
                                dsprintf("That doesn't seem to work.");
                        break;  
                case ACTION_FIX_VIEW:
                        fixview();
                        break;
                case ACTION_HEAL_PLAYER:
                        increase_hp(player, 1);
                        break;
                case ACTION_MOVE_MONSTER:
                        if(aqe)
                                move_monster(aqe->monster);
                        break;
                case ACTION_PLAYER_NEXTMOVE:
                        process_player_input();

                        i = 17 - pphy;
                        if(i <= 0)
                                i = 1;

                        if(!(game->tick % i)) {
                                if(perc(40+pphy))
                                        schedule_action(ACTION_HEAL_PLAYER, player);
                        }
                        
                        schedule_action_delayed(ACTION_PLAYER_NEXTMOVE, player, 0, 1);
                        break;
                case ACTION_NOTHING:
                        break;
                default:
                        fprintf(stderr, "DEBUG: %s:%d - Unknown action %d attempted!\n", __FILE__, __LINE__, aqe->action);
                        dsprintf("DEBUG: %s:%d - Unknown action %d attempted!\n", __FILE__, __LINE__, aqe->action);
                        break;
        }

        if(cf(ply, plx) & CF_HAS_DOOR_CLOSED) {
                open_door(ply, plx);
                you("open the door!");
                ply = oldy; plx = oldx;
        }

        return fullturn;
}

int get_next_free_action_slot()
{
        int i;

        for(i = 1; i < MAXACT; i++) {
                if(act[i].action == ACTION_FREESLOT)
                        return i;
        }

        return 0;
}

int schedule_action(int action, actor_t *actor)
{
        int i;

        i = get_next_free_action_slot();
        if(!i)
                die("fatal! no free slots in action queue!");

        act[i].action = action;
        act[i].tick = game->tick + actor->speed;
        act[i].actor = actor;
        //dsprintfc(COLOR_SKYBLUE, "Scheduled action %s at tick %d!", action_name[action], act[i].tick);

        return i;
}

int schedule_action_delayed(int action, actor_t *actor, obj_t *object, int delay)
{
        int i;

        i = get_next_free_action_slot();
        if(!i)
                die("fatal! no free slots in action queue!");

        act[i].action = action;
        act[i].tick = game->tick + actor->speed + delay;
        act[i].actor = actor;
        act[i].object = object;
        //dsprintfc(COLOR_SKYBLUE, "Scheduled delayed action %s at tick %d!", action_name[action], act[i].tick);

        return i;
}

int schedule_action_immediately(int action, actor_t *actor)
{
        int i;
        action_t a;

        i = get_next_free_action_slot();
        if(!i)
                die("fatal! no free slots in action queue!");

        a.action = action;
        a.tick = game->tick;


        //dsprintfc(COLOR_SKYBLUE, "Doing action immediately - %s at tick %d (game->tick = %d)!", action_name[action], act[i].tick, game->tick);
        do_action(&a);

        return i;
}

void unschedule_action(int index)
{
        act[index].action  = ACTION_FREESLOT;
        act[index].tick    = 0;
        act[index].monster = 0;
        act[index].object  = 0;
}

void schedule_monster(monster_t *m)
{
        int i;

        i = schedule_action(ACTION_MOVE_MONSTER, m);
        act[i].monster = m;

        //dsprintfc(COLOR_SKYBLUE, "Scheduled monster %s at tick %d", m->name, act[i].tick);
}

void unschedule_all_monsters()
{
        int i;

        for(i = 0; i < MAXACT; i++) {
                if(act[i].action == ACTION_MOVE_MONSTERS || act[i].action == ACTION_MOVE_MONSTER)
                        unschedule_action(i);
        }
}

/**
 * @brief Queue more than one instance of the same action.
 *
 * @param num How many instances.
 * @param action Which action to queue - see \ref group_actions "ACTION-defines" in gt.h
 */
void schedule_actionx(int num, int action, actor_t *actor)
{
        int i;

        for(i=0; i<num; i++)
                schedule_action(action, actor);
}

/**
 * @brief Add several, possibly different, actions to the action queue.
 *
 * @param first The first action to add (see \ref group_actions "ACTION-defines")
 * @param ... Additional actions to add. The last argument has to be ENDOFLIST.
 */
void queuemany(actor_t *actor, int first, ...)
{
        va_list args;
        int i;

        va_start(args, first);
        schedule_action(first, actor);

        i = va_arg(args, int);
        while(i != ENDOFLIST) {
                schedule_action(i, actor);
                i = va_arg(args, int);
        }
        va_end(args);
}

void process_autopickup()
{
        if(ci(ply, plx)) {
                if(ci(ply, plx) && ci(ply, plx)->gold) {
                        if(dsconfig.ap[OT_GOLD])
                                schedule_action_immediately(ACTION_PICKUP, player);
                }

                if(ci(ply, plx)->num_used > 0) {
                        if(ci(ply, plx)->num_used == 1) {
                                int slot;
                                obj_t *ob;

                                slot = get_first_used_slot(ci(ply, plx));
                                if(slot < 0)
                                        return;

                                ob = ci(ply, plx)->object[slot];
                                if(dsconfig.ap[ob->type] && !hasbit(ob->flags, OF_DONOTAP)) {
                                        schedule_action_immediately(ACTION_PICKUP, player);
                                }
                        }
                }
        }
}

/**
 * @brief Take a look at the player's current position, and tell him what he sees.
 *
 */
void look()
{
        //char *stairmat[] = { "stone", "wood", "bone", "marble", "metal" };

        if(cf(ply, plx) & CF_HAS_STAIRS_DOWN) {
                if(game->currentlevel == 0)
                        dsprintf("There is a portal to the underworld here!");
                else                                
                        dsprintf("There is a staircase leading down here.");
        }

        if(cf(ply, plx) & CF_HAS_STAIRS_UP) {
                if(game->currentlevel == 1) 
                        dsprintf("There is a portal to the outside world here!");
                else
                        dsprintf("There is a staircase leading up here.");
        }

        if(cf(ply, plx) & CF_HAS_EXIT) {
                int index;
                index = world->curlevel->c[ply][plx].exitindex;
                if(world->curlevel->exit[index].type == ET_EXIT) {
                        dsprintf("There is an exit leading to %s here.", areanames[world->curlevel->exit[index].location]);
                }
                if(world->curlevel->exit[index].type == ET_STAIRS_UP) {
                        dsprintf("There is a staircase leading up to %s here.", areanames[world->curlevel->exit[index].location]);
                }
                if(world->curlevel->exit[index].type == ET_STAIRS_DOWN) {
                        dsprintf("There is a staircase leading down to %s here.", areanames[world->curlevel->exit[index].location]);
                }
                if(world->curlevel->exit[index].type == ET_DOOR) {
                        dsprintf("There is a door leading to %s here.", areanames[world->curlevel->exit[index].location]);
                }
        }

        if(cf(ply, plx) & CF_HAS_FURNITURE) {
                if(cf(ply, plx) & CF_HASF_TABLE)
                        dsprintf("There is table here.");
                if(cf(ply, plx) & CF_HASF_CHAIR)
                        dsprintf("There is a chair here.");
                if(cf(ply, plx) & CF_HASF_FIRE)
                        dsprintf("There is a fire here!");

        }

        if(ci(ply, plx)) {
                if(ci(ply, plx) && ci(ply, plx)->gold) {
                        dsprintf("There is %d gold %s here.", ci(ply, plx)->gold, (ci(ply, plx)->gold > 1) ? "pieces" : "piece");
                }

                if(ci(ply, plx)->num_used > 0) {
                        if(ci(ply, plx)->num_used == 1) {
                                int slot;
                                obj_t *ob;

                                slot = get_first_used_slot(ci(ply, plx));
                                if(slot < 0)
                                        return;

                                ob = ci(ply, plx)->object[slot];
                                if(is_pair(ob))
                                        dsprintf("There is a pair of %s here.", ob->fullname);
                                else
                                        dsprintf("There is %s here.", a_an(ob->fullname));
                        }
                }

                if(ci(ply, plx)->num_used == 2) {
                        int slot, slot2;

                        slot  = get_first_used_slot(ci(ply, plx));
                        slot2 = get_next_used_slot_after(slot, ci(ply, plx));
                        if(slot < 0)
                                return;
                        if(slot2 < 0)
                                return;

                        dsprintf("There is %s and %s here.", a_an(ci(ply, plx)->object[slot]->fullname), a_an(ci(ply, plx)->object[slot2]->fullname));
                }

                if(ci(ply, plx)->num_used > 2) {
                        int i;

                        dsprintfc(COLOR_INFO, "There are several things here:");
                        for(i=0;i<52;i++) {
                                if(ci(ply, plx)->object[i])
                                        dsprintf("%s", a_an(ci(ply, plx)->object[i]->fullname));
                        }
                }
        }
}

void do_everything_at_tick(int tick)
{
        int i;

        for(i = 0; i < MAXACT; i++) {
                if(act[i].action >= 0) {
                        if(act[i].tick == tick) {
                                do_action(&act[i]);
                                unschedule_action(i);
                        }
                }
        }
}

void increase_ticks(int i)
{
        game->tick += i;
        //update_ticks();
}

/**
 * @brief Do a turn
 *
 */
void do_turn()
{
        int i, s;

        /*if(animate_only) {
                // Add cool animations here?!
                dsprintf("animating.....");
        } else {*/


        look_for_monsters();
        update_screen();

        // autoexplore

        if(game->dead)
                return;

        for(i = 0; i < 10; i++) {
                do_everything_at_tick(game->tick);
                look_for_monsters();
                increase_ticks(1);
                
                s = d(1, 10);
                inc_second(&game->t, s);    // replace with more precise time measuring? or keep it somewhat random, like it seems to be in the show?
                inc_second(&game->total, s);

                update_screen();
        }

        look();
        update_screen();
}

void process_player_input()
{
        int x, y, c, nx, ny, i;
        TCOD_key_t l;

        c = 0;
        while(!c)
                c = get_command();

        mapchanged   = false;
        animate_only = false;
        player->oldx = plx;
        player->oldy = ply;

        switch(c) {
                case CMD_QUIT:
                        schedule_action(ACTION_NOTHING, player);
                        game->dead = 1;
                        break;
                case CMD_DOWN:  schedule_action(ACTION_PLAYER_MOVE_DOWN, player);   break;
                case CMD_UP:    schedule_action(ACTION_PLAYER_MOVE_UP, player);     break;
                case CMD_LEFT:  schedule_action(ACTION_PLAYER_MOVE_LEFT, player);   break;
                case CMD_RIGHT: schedule_action(ACTION_PLAYER_MOVE_RIGHT, player);  break;
                case CMD_NW:    schedule_action(ACTION_PLAYER_MOVE_NW, player);     break;
                case CMD_NE:    schedule_action(ACTION_PLAYER_MOVE_NE, player);     break;
                case CMD_SW:    schedule_action(ACTION_PLAYER_MOVE_SW, player);     break;
                case CMD_SE:    schedule_action(ACTION_PLAYER_MOVE_SE, player);     break;
                case CMD_WIELDWEAR:
                                l = ask_char("Which item would you like to wield/wear?");
                                actiondata = (void *) get_object_from_letter(l.c, player->inventory);
                                schedule_action(ACTION_WIELDWEAR, player);
                                break;
                case CMD_UNWIELDWEAR:
                                l = ask_char("Which item would you like to remove/unwield?");
                                actiondata = (void *) get_object_from_letter(l.c, player->inventory);
                                schedule_action(ACTION_UNWIELDWEAR, player);
                                break;
                case CMD_DROP:
                                l = ask_char("Which item would you like to drop?");
                                actiondata = (void *) get_object_from_letter(l.c, player->inventory);
                                schedule_action(ACTION_DROP, player);
                                break;
                case CMD_LONGDOWN:
                                schedule_actionx(20, ACTION_PLAYER_MOVE_DOWN, player);
                                break;
                case CMD_LONGUP:
                                schedule_actionx(20, ACTION_PLAYER_MOVE_UP, player);
                                break;
                case CMD_LONGLEFT:
                                schedule_actionx(20, ACTION_PLAYER_MOVE_LEFT, player);
                                break;
                case CMD_LONGRIGHT:
                                schedule_actionx(20, ACTION_PLAYER_MOVE_RIGHT, player);
                                break;
                case CMD_TOGGLEFOV:
                                dsprintf("Setting all cells to visible.");
                                set_level_visited(world->curlevel);
                                break;
                case CMD_SPAWNMONSTER:
                                spawn_monster_at(ply+5, plx+5, ri(1, game->monsterdefs), world->curlevel->monsters, world->curlevel, 100);
                                break;
                case CMD_WIZARDMODE:
                                game->wizardmode = (game->wizardmode ? false : true);
                                dsprintf("Wizard mode %s!", game->wizardmode ? "on" : "off");
                                break;
                case CMD_SAVE:
                                save_game(game->savefile);
                                break;
                case CMD_LOAD:
                                if(!load_game(game->savefile, 1))
                                        dsprintf("Loading failed!");
                                else
                                        dsprintf("Loading successful!");
                                break;
                case CMD_DUMPOBJECTS:
                                dump_objects(world->curlevel->c[ply][plx].inventory);
                                break;
                case CMD_INCFOV:
                                player->viewradius++;
                                //world->out->lakelimit++;
                                //generate_terrain(1);
                                //dsprintf("lakelimit = %d", world->out->lakelimit);
                                break;
                case CMD_DECFOV:
                                player->viewradius--;
                                //world->out->lakelimit--;
                                //generate_terrain(1);
                                //dsprintf("lakelimit = %d", world->out->lakelimit);
                                break;
                case CMD_DUMPCOLORS:
                                for(x = 0;  x < 64; x++) {
                                        /*dsprintfwc(wstat, x, "This is color %d  ", x);
                                          wattron(wstat, A_BOLD);
                                          dsprintfwc(wstat, x, "This is BOLD color %d  ", x);
                                          wattroff(wstat, A_BOLD);*/
                                }
                                break;
                case CMD_FLOODFILL:
                                x = ri(11, world->curlevel->xsize);
                                y = ri(11, world->curlevel->ysize);
                                while(world->curlevel->c[y][x].type != CELL_FLOOR) {
                                        x = ri(11, world->curlevel->xsize);
                                        y = ri(11, world->curlevel->ysize);
                                }
                                dsprintf("floodfilling from %d, %d\n", y, x);
                                floodfill(world->curlevel, y, x);
                                break;
                case CMD_INVENTORY:
                                dump_objects(player->inventory);
                                break;
                case CMD_PICKUP:
                                schedule_action(ACTION_PICKUP, player);
                                break;
                case CMD_DESCEND:
                                if(hasbit(cf(ply,plx), CF_HAS_STAIRS_DOWN)) {
                                        schedule_action(ACTION_GO_DOWN_STAIRS, player);
                                        schedule_action(ACTION_FIX_VIEW, player);
                                } else {
                                        dsprintf("You can't go up here!");
                                }
                                break;
                case CMD_ASCEND:
                                if(hasbit(cf(ply,plx), CF_HAS_STAIRS_UP)) {
                                        schedule_action(ACTION_GO_UP_STAIRS, player);
                                        schedule_action(ACTION_FIX_VIEW, player);
                                } else {
                                        dsprintf("You can't go up here!");
                                }
                                break;
                case CMD_USE_EXIT:
                                if(hasbit(cf(ply, plx), CF_HAS_EXIT)) {
                                        schedule_action(ACTION_USE_EXIT, player);
                                        schedule_action(ACTION_FIX_VIEW, player);
                                } else {
                                        dsprintf("There is no exit here!");
                                }
                                break;
                case CMD_REST:
                                break;
                case CMD_PATHFINDER:
                                nx = plx; ny = ply;

                                TCOD_path_compute(player->path, player->x, player->y, player->x + (ri(-30,30)), player->y + (ri(-30,30)));
                                for(i = 0; i < TCOD_path_size(player->path); i++) {
                                        TCOD_path_get(player->path, i, &x, &y);
                                        //world->curlevel->c[y][x].backcolor = TCOD_light_blue;
                                        // and let's move!
                                        if(y > ny) { // moving downward
                                                if(x > nx)
                                                        schedule_action(ACTION_PLAYER_MOVE_SE, player);
                                                if(x < nx)
                                                        schedule_action(ACTION_PLAYER_MOVE_SW, player);
                                                if(x == nx)
                                                        schedule_action(ACTION_PLAYER_MOVE_DOWN, player);
                                        }

                                        if(y < ny) {
                                                if(x > nx)
                                                        schedule_action(ACTION_PLAYER_MOVE_NE, player);
                                                if(x < nx)
                                                        schedule_action(ACTION_PLAYER_MOVE_NW, player);
                                                if(x == nx)
                                                        schedule_action(ACTION_PLAYER_MOVE_UP, player);
                                        }

                                        if(y == ny) {
                                                if(x > nx)
                                                        schedule_action(ACTION_PLAYER_MOVE_RIGHT, player);
                                                if(x < nx)
                                                        schedule_action(ACTION_PLAYER_MOVE_LEFT, player);
                                        }
                                        nx = x; ny = y;
                                }
                                break;
                case CMD_INCTIME:
                                c = 5000000;
                                while(c--) {
                                        inc_second(&game->t, 1);
                                };
                                draw_map(world->curlevel);
                                draw_wstat();
                                update_screen();

                                break;
                default:
                                break;
        }
}

void game_loop()
{
        while(!game->dead) {
                do_turn();
        };
}

/**
 * @brief Catch a signal and perform an action (currently, that action is to exit).
 */
void catchsignal()
{
        game->dead = true;
        shutdown_display();
        shutdown_ds();
        fprintf(stderr, "Caught signal - exiting.\n");
        exit(0);
}

int main(int argc, char *argv[])
{
        char messagefilename[50];

        signal(SIGINT,  catchsignal);
        signal(SIGKILL, catchsignal);

        if(!setlocale(LC_ALL, ""))
                die("couldn't set locale.");

        init_variables();

        get_version_string(game->version);
        printf("%s v%s\n", GAME_NAME, game->version);

        parse_commandline(argc, argv);

        if(!loadgame) {
                init_objects();
                printf("Reading data files...\n");
                if(parse_data_files(0))
                        die("Couldn't parse data files.");
                start_autopickup();
        }

        if(loadgame) {
                init_player();
                parse_data_files(ONLY_CONFIG);
                if(!load_game(game->savefile, 0))
                        die("Couldn't open file %s\n", game->savefile);

                sprintf(messagefilename, "%s/messages.%d.dssave", SAVE_DIRECTORY, game->seed);
                messagefile = fopen(messagefilename, "a");
                
                init_display();
                
                // these next should be loaded by load_game?!
                world->cmap = world->area[game->currentlevel].c;
                world->curlevel = &world->area[game->currentlevel];
        } else {
                sprintf(messagefilename, "%s/messages.%d.dssave", SAVE_DIRECTORY, game->seed);
                messagefile = fopen(messagefilename, "a");

                init_level(world->out);
                generate_world();

                init_display();
                init_player();
                player->inventory = init_inventory();


                world->curlevel = &world->area[AREA_COLLINWOOD_MAIN_FLOOR];
                world->cmap = world->curlevel->c;
                game->currentlevel = 1;
                game->context = CONTEXT_INSIDE;

                // then move down a level...
                // move_player_to_stairs_down(0);
                // do_action(ACTION_GO_DOWN_STAIRS);
                fixview();
        }

        init_commands();

        player->path = TCOD_path_new_using_map(world->curlevel->map, 1.0f);
        draw_map(world->curlevel);
        draw_wstat();
        initial_update_screen();

        schedule_action_delayed(ACTION_PLAYER_NEXTMOVE, player, 0, 1);

        game_loop();

        shutdown_display();
        shutdown_ds();

        return 0;
}
// vim: fdm=syntax guifont=Terminus\ 8
