/*
 * Dark Shadows - The Roguelike
 *
 * dealing with time and date
 *
 * Copyright 2012 Rolf Klausen
 */

#ifndef _DSTIME_H
#define _DSTIME_H

typedef struct {
        int second, minute, hour;
        int day, month, year;
} dstime;

extern char *monthstring[];

void inc_second(dstime *t, int i);

#endif
