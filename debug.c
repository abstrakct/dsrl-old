/*
 * Dark Shadows - The Roguelike
 *
 * Various debugging stuff.
 *
 * Copyright 2011 Rolf Klausen
 */
#include <stdio.h>
#include <stdbool.h>

#include "npc-names.h"
#include "objects.h"
#include "actor.h"
#include "monsters.h"
#include "utils.h"
#include "world.h"
#include "datafiles.h"
#include "display.h"
#include "debug.h"
#include "dstime.h"
#include "dsrl.h"

extern action_t    *act;

void dump_monsterdefs()
{
    monster_t *m, *n;
    int i;

    n = monsterdefs->head;
    for(i=0;i<monsterdefs->head->x;i++) {
        m = n->next;
        dsprintf("%s\t%c\nstr\t%d\tphy\t%d\tintl\t%d\twis\t%d\tdex\t%d\tcha\t%d\n", m->name, m->c, m->attr.str, m->attr.phy, m->attr.intl, m->attr.wis, m->attr.dex, m->attr.cha);
        dsprintf("hp\t%d\t\tlevel\t%d\tspeed\t%.1f\n", m->hp, m->level, m->speed);
        dsprintf("Can use weapon: %s\tCan use armor: %s\tCan have gold: %s\n", m->flags & MF_CANUSEWEAPON ? "Yes" : "No", m->flags & MF_CANUSEARMOR ? "Yes" : "No", m->flags & MF_CANHAVEGOLD ? "Yes" : "No");
        dsprintf("\n");
        n = m;
    }
}

void dump_monsters(monster_t *list)
{
    monster_t *m;

    m = list->next;
    while(m) {
        dsprintf("monsterdump: %s\n", m->name);
        m = m->next;
    }
}

void dump_objects(inv_t *i)
{
    obj_t *o;
    int j;

    if(!i) {
        dsprintf("No objects here!");
        return;
    }

    for(j = 0; j < 52; j++) {
        o = i->object[j];
        if(o) {
            dsprintf("\n");
            dsprintf("OID:      %d\tBasename: %s\tType:     %s", o->oid, o->basename, otypestrings[o->type]);
            if(o->type == OT_GOLD)
                dsprintf("Amount:   %d\n", o->quantity);
            if(is_armor(o))
                dsprintf("AC:       %d\n", o->ac);
            dsprintf("Attack modifier:%s%d\n", (o->attackmod >= 0 ? " +" : " "), o->attackmod);
            dsprintf("Damage modifier:%s%d\n", (o->damagemod >= 0 ? " +" : " "), o->damagemod);
            dsprintf("Unique:   %s\n", is_unique(o) ? "yes" : "no");
            if(is_weapon(o))
                dsprintf("Damage:   %dd%d\n", o->dice, o->sides);

            dsprintf("\n");
        }
    }

}

void dump_action_queue()
{
    int i;
    struct actionqueue *tmp;

    tmp = aq;
    i = 0;
    while(tmp) {
        dsprintf("item %d\taction %d\tnum %d\n", i, tmp->action, tmp->num);
        tmp = tmp->next; 
        i++;
    }
}

void dump_scheduled_actions()
{
    printf("\nSCHEDULED ACTIONS (tick = %d)\n", game->tick);

    int i;

    for(i = 0; i < MAXACT; i++) {
        if(act[i].action != ACTION_FREESLOT) {
            printf("act[%d]", i);
            switch(act[i].action) {
                case ACTION_NOTHING: printf("\tNothing"); break;
                case ACTION_PLAYER_MOVE_LEFT: printf("\tPlayer move left"); break;
                case ACTION_PLAYER_MOVE_RIGHT: printf("\tPlayer move right"); break;
                case ACTION_PLAYER_MOVE_UP: printf("\tPlayer move up"); break;
                case ACTION_PLAYER_MOVE_DOWN: printf("\tPlayer move down"); break;
                case ACTION_PLAYER_MOVE_NW: printf("\tPlayer move NW"); break;
                case ACTION_PLAYER_MOVE_NE: printf("\tPlayer move NE"); break;
                case ACTION_PLAYER_MOVE_SW: printf("\tPlayer move SW"); break;
                case ACTION_PLAYER_MOVE_SE: printf("\tPlayer move SE"); break;
                case ACTION_PICKUP: printf("\tPick up"); break;
                case ACTION_ATTACK: printf("\tAttack"); break;
                case ACTION_MOVE_MONSTERS: printf("\tMove monsters"); break;
                case ACTION_ENTER_DUNGEON: printf("\tEnter dungeon"); break;
                case ACTION_GO_DOWN_STAIRS: printf("\tGo down stairs"); break;
                case ACTION_GO_UP_STAIRS: printf("\tGo up stairs"); break;
                case ACTION_FIX_VIEW: printf("\tFix view"); break;
                case ACTION_WIELDWEAR: printf("\tWield/wear"); break;
                case ACTION_UNWIELDWEAR: printf("\tUnwield/unwear"); break;
                case ACTION_HEAL_PLAYER: printf("\tHeal player"); break;
                case ACTION_DROP: printf("\tDrop"); break;
                case ACTION_USE_EXIT: printf("\tUse exit"); break;
                case ACTION_MOVE_MONSTER: printf("\tMove monster"); break;
                case ACTION_PLAYER_NEXTMOVE: printf("\tPlayer next move"); break;
                case ACTION_MOVE_NPC: printf("\tMove NPC"); break;
                default: printf("\tunknown?!"); break;
            }
            printf("\t\ttick = %d\tmonster = %s\tobject = %s\tactor = %s\tgain = %d\n", act[i].tick, act[i].monster ? "yes" : "no", act[i].object ? "yes" : "no", act[i].actor ? "yes" : "no", act[i].gain);
        }
    }
}
// vim: fdm=syntax guifont=Terminus\ 8
