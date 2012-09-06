/*
 * Dark Shadows - The Roguelike
 *
 * dstime.c - handling time, date, etc.
 *
 * Copyright 2012 Rolf Klausen
 */

#include "objects.h"
#include "utils.h"
#include "dstime.h"

char *monthstring[] = {
        0, "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"
};

int monthdays[]  = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

void inc_year(dstime *t, int i)
{
        t->year += i;
        if(t->year > 9999) {
                dsprintf("I think you may have played long enough now! How about some fresh air?");
        }
}

void inc_month(dstime *t, int i)
{
        t->month += i;
        if(t->month > 12) {
                inc_year(t, 1);
                t->month = 1;
        }
}

void inc_day(dstime *t, int i)
{
        t->day += i;
        if(t->day >= monthdays[t->month]) {
                inc_month(t, 1);
                t->day = 1;
        }
}

void inc_hour(dstime *t, int i)
{
        t->hour += i;
        if(t->hour >= 24) {
                inc_day(t, 1);
                t->hour -= 24;
        }
}

void inc_minute(dstime *t, int i)
{
        t->minute += i;
        if(t->minute >= 60) {
                inc_hour(t, 1);
                t->minute -= 60;
        }
}

void inc_second(dstime *t, int i)
{
        t->second += i;
        if(t->second >= 60) {
                inc_minute(t, 1);
                t->second -= 60;
        }
}
// vim: fdm=syntax guifont=Terminus\ 8
