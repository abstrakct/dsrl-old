/*
 * Dark Shadows - The Roguelike
 *
 * Copyright 2011 Rolf Klausen
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#ifdef GT_USE_NCURSES
#include <curses.h>
#endif

#include "objects.h"
#include "actor.h"
#include "monsters.h"
#include "utils.h"
#include "world.h"
#include "display.h"
#include "dsrl.h"


void *garbage[MAX_GARBAGE];
int garbageindex;

int ri(int a, int b) 
{
        int result;
        result = (a + (rand() % (b-a+1)));

        return result;
}

void get_version_string(char *s)
{
        sprintf(s, "%d.%d.%d", GT_VERSION_MAJ, GT_VERSION_MIN, GT_VERSION_REV);
}

void die(char *m, ...)
{
        va_list argp;
        char s[1000];
        char s2[1020];

        sprintf(s2, "FATAL ERROR: ");

        va_start(argp, m);
        vsprintf(s, m, argp);
        va_end(argp);

        strcat(s2, s);
        fprintf(stderr, "%s", s2);

        shutdown_display();
        shutdown_ds();
        exit(1);
}

int dice(int num, int sides, signed int modifier)
{
        int i, result;

        result = modifier;
        for(i=0;i<num;i++) {
                result += 1 + (rand() % sides);
        }

        return result;
}

int perc(int i)
{
        int x;

        if(i >= 100)
                return true;

        x = ri(1, 100);
        if(x <= i)
                return true;
        else
                return false;
}

void *dsmalloc(size_t size)
{
        void *p;

        p = malloc(size);
        if(!p)
                die("Memory allocation in dsmalloc for size %d failed! Exiting.\n", (int) size);

        memset(p, 0, size);
        garbage[garbageindex] = p;
        garbageindex++;

        return p;
}

void *dscalloc(size_t num, size_t size)
{
        void *p;

        p = calloc(num, size);
        if(!p)
                die("calloc %d * %d (total %d) in dscalloc failed! Exiting.\n", (int) num, (int) size, (int) (num*size));

        memset(p, 0, num*size);
        garbage[garbageindex] = p;
        garbageindex++;

        return p;
}

void **dsmalloc2d(int y, int x, size_t size)
{
        void **p;
        int i;
        
        p = dsmalloc(y * size);
        for(i = 0; i < y; i++)
                p[i] = dsmalloc(x * size);

        return p;
}

void dsfree(void *ptr)
{
        int i;

        for(i=0;i<MAX_GARBAGE;i++)
                if(garbage[i] == ptr)
                        break;

        garbage[i] = NULL;
        free(ptr);
}

void you(char *fmt, ...)
{
        va_list argp;
        char s[1000];
        char s2[1020];

        sprintf(s2, "You ");

        va_start(argp, fmt);
        vsprintf(s, fmt, argp);
        va_end(argp);

        strcat(s2, s);
        mess(s2);
}

void youc(int color, char *fmt, ...)
{
        va_list argp;
        char s[1000];
        char s2[1020];

        sprintf(s2, "You ");

        va_start(argp, fmt);
        vsprintf(s, fmt, argp);
        va_end(argp);

        strcat(s2, s);
        messc(color, s2);
}

void yousee(char *fmt, ...)
{
        va_list argp;
        char s[1000];
        char s2[1020];

        sprintf(s2, "You see ");

        va_start(argp, fmt);
        vsprintf(s, fmt, argp);
        va_end(argp);

        strcat(s2, s);
        mess(s2);
}

void dsprintf(char *fmt, ...)
{
        va_list argp;
        char s[1000];

        va_start(argp, fmt);
        vsprintf(s, fmt, argp);
        va_end(argp);

        mess(s);
}

void dsprintfc(int color, char *fmt, ...)
{
        va_list argp;
        char s[1000];

        va_start(argp, fmt);
        vsprintf(s, fmt, argp);
        va_end(argp);

        messc(color, s);
}

char ask_char(char *question)
{
        char c;

        dsprintf(question);
        update_screen();
        c = dsgetch();
        return c;
}

char ask_for_hand()
{
        char c;
        
        c = 0;

        while(1) {
                dsprintf("Which hand - (l)eft or (r)ight?");
                update_screen();
                c = dsgetch();
//fprintf(stderr, "DEBUG: %s:%d - you pressed key with decimal value %d\n", __FILE__, __LINE__, c);
                if(c == 13 || c == 27)         // ENTER or ESCAPE
                        return 0;
                else if(c == 'l' || c == 'r')
                        return c;
                else
                        dsprintf("Only (l)eft or (r)ight, please.");
        }

}

bool yesno(char *fmt, ...)
{
        va_list argp;
        char s[1000];
        char c;

        va_start(argp, fmt);
        vsprintf(s, fmt, argp);
        va_end(argp);

        strcat(s, " (y/n)?");
        mess(s);

        update_screen();
        c = dsgetch();
        if(c == 'y' || c == 'Y')
                return true;
        if(c == 'n' || c == 'N')
                return false;

        return false;
}

void more()
{
        char c;

        dsprintfc(COLOR_WHITE, "-- more --");
        while(1) {
                c = dsgetch();
                if(c == 13 || c == 32) {
                        delete_last_message();
                        return;
                }
        }
}

#ifdef GT_USE_NCURSES
void dsprintfwc(WINDOW *win, int color, char *fmt, ...)
{
        va_list argp;
        char s[1000];

        va_start(argp, fmt);
        vsprintf(s, fmt, argp);
        va_end(argp);

        wattron(win, COLOR_PAIR(color));
        wprintw(win, s);
        wattroff(win, COLOR_PAIR(color));

        //messc(color, s);
}
#endif

void uppercase(char *s)
{
        int i;

        for(i=0;i<strlen(s);i++) {
                if(s[i] >= 97 && s[i] <= 122) {
                        s[i] -= 32;
                        return;
                }
        }
}

char *Upper(char *s)
{
        if(s[0] >= 97 && s[0] <= 122)
                s[0] -= 32;

        return s;
}

char *a_an(char *s)
{
        static char ret[4];
        char *test;

        test = dsmalloc((strlen(s) + 5) * sizeof(char));

        if(s[0] == 'a' || s[0] == 'A' ||
           s[0] == 'e' || s[0] == 'A' ||
           s[0] == 'i' || s[0] == 'I' ||
           s[0] == 'o' || s[0] == 'O' ||
           s[0] == 'u' || s[0] == 'U' ||
           s[0] == 'y' || s[0] == 'Y')
                strcpy(ret, "an");
        else
                strcpy(ret, "a");

        sprintf(test, "%s %s", ret, s);
        return test;
}
