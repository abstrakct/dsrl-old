/*
 * Dark Shadows - The Roguelike
 *
 * Various debugging stuff.
 *
 * Copyright 2011 Rolf Klausen
 */
#include <stdio.h>
#include <stdbool.h>

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
// vim: fdm=syntax guifont=Terminus\ 8
