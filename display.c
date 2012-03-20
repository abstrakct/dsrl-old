/*
 * Dark Shadows - The Roguelike
 *
 * This file deals with diplaying output!
 *
 * Copyright 2011 Rolf Klausen
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>
#include <libtcod/libtcod.h>

#include "objects.h"
#include "actor.h"
#include "monsters.h"
#include "utils.h"
#include "world.h"
#include "datafiles.h"
#include "display.h"
#include "dstime.h"
#include "dsrl.h"

extern int maxmess;

bool blocks_light(int y, int x)
{
        level_t *l = world->curlevel;

        if(hasbit(l->c[y][x].flags, CF_HAS_DOOR_CLOSED))
                return true;

        /*
        if(l->c[y][x].type == AREA_FOREST || l->c[y][x].type == AREA_CITY || l->c[y][x].type == AREA_VILLAGE) {    // trees and houses can be "see through" (e.g. if they are small)
                if(perc(20))
                        return false;
                else
                        return true;
        }
        */
        switch(l->c[y][x].type) {
                case CELL_NOTHING:
                case CELL_WALL:
                       return true;
                default:       
                       return false;
        }

        // shouldn't be reached...
        return false;
}

void domess()
{
        int i;

        TCOD_console_set_default_foreground(game->messages.c, TCOD_white);
        TCOD_console_print_frame(game->messages.c, 0, 0, game->messages.w, game->messages.h - 1, true, TCOD_BKGND_NONE, "Messages");
        currmess++;
        for(i = maxmess-1; i > 0; i--) {
                TCOD_console_set_default_foreground(game->messages.c, messages[i].color);
                TCOD_console_print(game->messages.c, 1, i, "%s", messages[i].text);
        }
        TCOD_console_blit(game->messages.c, 0, 0, game->messages.w, game->messages.h, NULL, game->messages.x, game->messages.y, 1.0, 1.0);
        TCOD_console_flush();

        fprintf(messagefile, "%s\n", messages[currmess-1].text);
}

void scrollmessages()
{
        int i;

        if (currmess >= maxmess) {
                currmess = maxmess - 1;
                for(i = 0; i <= currmess; i++) {
                        messages[i].color = messages[i+1].color;
                        strcpy(messages[i].text, messages[i+1].text);
                }
        }
}

void mess(char *message)
{
        //if(!strcmp(message, messages[currmess-1].text))
                //return;

        scrollmessages();
        messages[currmess].color = TCOD_white;
        strcpy(messages[currmess].text, message);
        domess();
}

void delete_last_message()
{
        messages[currmess-1].color = TCOD_white;
        messages[currmess-1].text[0] = '\0';
        domess();
}

void messc(TCOD_color_t color, char *message)
{
        //if(!strcmp(message, messages[currmess-1].text))
                //return;

        scrollmessages();
        messages[currmess].color = color;
        strcpy(messages[currmess].text, message);
        domess();
}

/*
void draw_wstat()
{
        obj_t *o;
        int i, j;
        int color;

        werase(wleft);
        werase(wstat);
        box(wleft, ACS_VLINE, ACS_HLINE);                                                                                                                                                                                                                                                                         
        box(wstat, ACS_VLINE, ACS_HLINE);                                                                                                                                                                                                                                                                         

        mvwprintw(wleft, 1, 1, "Name:   %s", player->name);
        mvwprintw(wleft, 2, 1, "Time:   %02d:%02d - %s %d, %d", game->t.hour, game->t.minute, monthstring[game->t.month], game->t.day, game->t.year);
        mvwprintw(wleft, 3, 1, "Weapon: %s", player->weapon ? player->weapon->fullname : "bare hands");
        //mvwprintw(wleft, 3, 1, "y,x     %d,%d", ply, plx);
        //mvwprintw(wleft, 4, 1, "(py,px) (%d,%d)", ppy, ppx);
        mvwprintw(wleft, 5, 1, "viewradius: %d", player->viewradius);
        if(player->hp >= (player->maxhp/4*3))
                color = COLOR_PAIR(COLOR_GREEN);
        else if(player->hp >= (player->maxhp/4) && player->hp < (player->maxhp/4*3))
                color = COLOR_PAIR(COLOR_YELLOW);
        else if(player->hp < (player->maxhp/4))
                color = COLOR_PAIR(COLOR_RED);
        mvwprintw(wleft, 6, 1, "HP:");
        wattron(wleft, color);
        mvwprintw(wleft, 6, 5, "%d/%d (%.1f%%)", player->hp, player->maxhp, ((float)(100/(float)player->maxhp) * (float)player->hp));
        wattroff(wleft, color);
        mvwprintw(wleft, 7, 1, "Player level: %d", player->level);
        mvwprintw(wleft, 8, 1, "AC: %d", player->ac);
        mvwprintw(wleft, 9, 1, "Dungeon level: %d (out of %d)", game->currentlevel, game->createdareas);
        mvwprintw(wleft, 10, 1, "STR:   %d", player->attr.str);
        mvwprintw(wleft, 11, 1, "DEX:   %d", player->attr.dex);
        mvwprintw(wleft, 12, 1, "PHY:   %d", player->attr.phy);
        mvwprintw(wleft, 13, 1, "INT:   %d", player->attr.intl);
        mvwprintw(wleft, 14, 1, "WIS:   %d", player->attr.wis);
        mvwprintw(wleft, 15, 1, "CHA:   %d", player->attr.cha);
        mvwprintw(wleft, 16, 1, "XP:    %d", player->xp);
        mvwprintw(wleft, 17, 1, "Level: %d", player->level);


        mvwprintw(wstat, 1, 1, "== INVENTORY ==");
        mvwprintw(wstat, 2, 1, "Gold: %d", player->inventory->gold);
        
        i = 3;
        for(j = 0; j < 52; j++) {
                if(player->inventory->object[j]) {
                        //o = get_object_from_letter(slot_to_letter(j), player->inventory);
                        o = player->inventory->object[j];
                        if(is_worn(o)) {
                                mvwprintw(wstat, i, 1, "%c)   %s %s", slot_to_letter(j), a_an(o->fullname), is_bracelet(o) ? (o == pw_leftbracelet ? "[<]" : "[>]") : "\0");
                                wattron(wstat, COLOR_PAIR(COLOR_GREEN));
                                mvwprintw(wstat, i, 4, "*"); 
                                wattroff(wstat, COLOR_PAIR(COLOR_GREEN));
                        } else {
                                mvwprintw(wstat, i, 1, "%c)   %s", slot_to_letter(j), a_an(o->fullname));
                        }
                        i++;
                }
        }
}
*/

void clear_map_to_invisible(level_t *l)
{
        int x, y;

        for(y = ppy; y < (ppy+game->map.h); y++) {
                for(x = ppx; x < (ppx+game->map.w); x++) {
                        if(x >= 0 && y >= 0 && x < l->xsize && y < l->ysize)
                                l->c[y][x].visible = 0;
                }
        }
}

void clear_map_to_unlit(level_t *l)
{
        int x, y;

        for(y = 0; y < l->ysize; y++) {
                for(x = 0; x < l->xsize; x++) {
                        clearbit(l->c[y][x].flags, CF_LIT);
                }
        }
}

/*
 * The next two functions are about FOV.
 * Stolen/adapted from http://roguebasin.roguelikedevelopment.org/index.php/Eligloscode
 */

void dofov(actor_t *actor, level_t *l, float x, float y)
{
        int i;
        float ox, oy;

        ox = (float) actor->x + 0.5f;
        oy = (float) actor->y + 0.5f;

        for(i = 0; i < actor->viewradius; i++) {
                if((int)oy >= 0 && (int)ox >= 0 && (int)oy < l->ysize && (int)ox < l->xsize) {
                        l->c[(int)oy][(int)ox].visible = 1;
                        setbit(l->c[(int)oy][(int)ox].flags, CF_VISITED);
                        if(blocks_light((int) oy, (int) ox)) {
                                return;
                        }/* else {  //SCARY MODE! 
                                if(perc((100-actor->viewradius)/3))
                                        return;
                        }*/


                        ox += x;
                        oy += y;
                }
        }
}

void FOV(actor_t *a, level_t *l)
{
        float x, y;
        int i;
        //signed int tmpx,tmpy;

        // if dark area
        clear_map_to_invisible(l);

        for(i = 0; i < 360; i++) {
                x = cos((float) i * 0.01745f);
                y = sin((float) i * 0.01745f);
                dofov(a, l, x, y);
        }
}

void dofovlight(actor_t *actor, level_t *l, float x, float y)
{
        int i;
        float ox, oy;

        ox = (float) actor->x + 0.5f;
        oy = (float) actor->y + 0.5f;


        //dsprintf("\tentering dofovlight");
        for(i = 0; i < (actor->viewradius/2); i++) {       // TODO: add a lightradius in actor_t, calculate it based on stuff
                if((int)oy >= 0 && (int)ox >= 0 && (int)oy < l->ysize && (int)ox < l->xsize) {
                        //dsprintf("\t\tchecking cell %d,%d", (int)oy, (int)ox);
                        if(hasbit(l->c[(int)oy][(int)ox].flags, CF_LIT))
                                return;

                        if(l->c[(int)oy][(int)ox].type == CELL_WALL) {
                                setbit(l->c[(int)oy][(int)ox].flags, CF_LIT);
                        }

                        if(blocks_light((int) oy, (int) ox)) {
                                //dsprintf("cell %d,%d blocks light", (int)oy, (int)ox);
                                return;
                        }

                        ox += x;
                        oy += y;
                }
        }
}

void FOVlight(actor_t *a, level_t *l)
{
        float x, y;
        int i;

        //dsprintf("entering FOVlight..");
        clear_map_to_unlit(l);
        for(i = 0; i < 360; i++) {
                x = cos((float) i * 0.01745f);
                y = sin((float) i * 0.01745f);
                //fprintf(stderr, "DEBUG: %s:%d - now going to dofovlight i = %d y = %.4f x = %.4f\n", __FILE__, __LINE__, i, y, x);
                dofovlight(a, l, x, y);
        }
}

void draw_map(level_t *level)
{
        int i,j, slot;
        int dx, dy;  // coordinates on screen!
        TCOD_color_t color;

        FOV(player, level);
        if(game->context == CONTEXT_INSIDE)
                FOVlight(player, level);     // only necessary inside

        /*
         * in this function, (j,i) are the coordinates on the map,
         * dx,dy = coordinates on screen.
         * so, player->py/px describes the upper left corner of the map
         */
        for(i = ppx, dx = 1; i < (ppx + game->map.w - 2); i++, dx++) {
                for(j = ppy, dy = 1; j < (ppy + game->map.h - 2); j++, dy++) {
                        if(j < level->ysize && i < level->xsize) {
                                if(hasbit(level->c[j][i].flags, CF_VISITED)) {
                                        color = cc(j,i);

                                        if(hasbit(level->c[j][i].flags, CF_LIT)) {
                                                color = level->c[j][i].litcolor;
                                        }

                                        dsmapaddch(dy, dx, color, mapchars[(int) level->c[j][i].type]);

                                        if(level->c[j][i].inventory) {
                                                if(level->c[j][i].inventory->gold > 0) {
                                                        dsmapaddch(dy, dx, TCOD_yellow, objchars[OT_GOLD]);
                                                } else {                                                         // TODO ADD OBJECT COLORS!!!
                                                        slot = get_first_used_slot(level->c[j][i].inventory);
                                                        if(level->c[j][i].inventory->num_used > 0 && slot >= 0 && level->c[j][i].inventory->object[slot]) {
                                                                color = level->c[j][i].inventory->object[slot]->color;
                                                                dsmapaddch(dy, dx, color, objchars[level->c[j][i].inventory->object[slot]->type]);
                                                        }
                                                }
                                        }

                                        if(hasbit(level->c[j][i].flags, CF_HAS_DOOR_CLOSED))
                                                dsmapaddch(dy, dx, color, '+');
                                        else if(hasbit(level->c[j][i].flags, CF_HAS_DOOR_OPEN))
                                                dsmapaddch(dy, dx, color, '\'');
                                        else if(hasbit(level->c[j][i].flags, CF_HAS_STAIRS_DOWN))
                                                dsmapaddch(dy, dx, TCOD_white, '>');
                                        else if(hasbit(level->c[j][i].flags, CF_HAS_STAIRS_UP))
                                                dsmapaddch(dy, dx, TCOD_white, '<');
                                        else if(hasbit(level->c[j][i].flags, CF_HAS_EXIT)) {
                                                int index;
                                                index = level->c[j][i].exitindex;
                                                if(level->exit[index].type == ET_EXIT)
                                                        dsmapaddch(dy, dx, TCOD_white, '^');
                                                if(level->exit[index].type == ET_STAIRS_UP)
                                                        dsmapaddch(dy, dx, TCOD_white, '|');
                                                if(level->exit[index].type == ET_STAIRS_DOWN)
                                                        dsmapaddch(dy, dx, TCOD_white, '>');
                                                if(level->exit[index].type == ET_DOOR)
                                                        dsmapaddch(dy, dx, TCOD_white, '+');
                                        } 
                                }


                                if(level->c[j][i].visible && level->c[j][i].monster /*&& actor_in_lineofsight(player, level->c[j][i].monster)*/)
                                        dsmapaddch(dy, dx, TCOD_red, (char) level->c[j][i].monster->c);

                        if(j == ply && i == plx)
                                dsmapaddch(dy, dx, TCOD_blue, '@');
                        }
                }
        }

}

void dsmapaddch(int y, int x, TCOD_color_t color, char c)
{
        TCOD_console_put_char_ex(game->map.c, x, y, c, color, TCOD_black);
}

void initial_update_screen()
{
        dsprintf("Welcome to %s v%s", GAME_NAME, game->version);
        dsprintf("Welcome to %s v%s", GAME_NAME, game->version);
        //domess();
        update_screen();
}

void update_screen()
{
        //TCOD_console_rect(game->map.c, game->map.x, game->map.y, game->map.w, game->map.h, true, TCOD_BKGND_NONE);
        TCOD_console_print_frame(game->map.c, 0, 0, game->map.w, game->map.h, true, TCOD_BKGND_NONE, "Map");
        TCOD_console_print_frame(game->left.c, 0, 0, game->left.w, game->left.h, true, TCOD_BKGND_NONE, "You");
        TCOD_console_print_frame(game->right.c, 0, 0, game->right.w - 2, game->right.h, true, TCOD_BKGND_NONE, "Inventory");

        draw_map(world->curlevel);
        TCOD_console_blit(game->map.c, 0, 0, game->map.w, game->map.h, NULL, game->map.x, game->map.y, 1.0, 1.0);
        TCOD_console_blit(game->messages.c, 0, 0, game->messages.w, game->messages.h, NULL, game->messages.x, game->messages.y, 1.0, 1.0);
        TCOD_console_blit(game->left.c, 0, 0, game->left.w, game->left.h, NULL, game->left.x, game->left.y, 1.0, 1.0);
        TCOD_console_blit(game->right.c, 0, 0, game->right.w, game->right.h, NULL, game->right.x, game->right.y, 1.0, 1.0);

        TCOD_console_flush();
}

void draw_wstat()
{
}

TCOD_key_t dsgetch()
{
        TCOD_key_t key;

        key = TCOD_console_wait_for_keypress(true);

        return key;
}

void init_display()
{ 
        /* font selection code is stolen from brogue! */
	char font[60];
	int fontsize = -1;
	
	int screenwidth, screenheight;
	int fontwidths[13] = {112, 128, 144, 160, 176, 192, 208, 224, 240, 256, 272, 288, 304}; // widths of the font graphics (divide by 16 to get individual character width)
	int fontheights[13] = {176, 208, 240, 272, 304, 336, 368, 400, 432, 464, 496, 528, 528}; // heights of the font graphics (divide by 16 to get individual character height)

	TCOD_sys_get_current_resolution(&screenwidth, &screenheight);

	// adjust for title bars and whatever -- very approximate, but better than the alternative
	screenwidth -= 6;
	screenheight -= 48;

	if (fontsize < 1 || fontsize > 13) {
		for (fontsize = 13; fontsize > 1 && (fontwidths[fontsize - 1] * COLS / 16 >= screenwidth || fontheights[fontsize - 1] * ROWS / 16 >= screenheight); fontsize--);
	}

	sprintf(font, "fonts/ds.png");
	//sprintf(font, "fonts/font-%i.png", fontsize);
        TCOD_console_set_custom_font(font, /*TCOD_FONT_TYPE_GREYSCALE |*/ TCOD_FONT_LAYOUT_ASCII_INROW, 16, 16);

        TCOD_console_init_root(COLS, ROWS, GAME_NAME, false, TCOD_RENDERER_SDL);
	//TCOD_console_map_ascii_codes_to_font(0, 255, 0, 0);
	TCOD_console_set_keyboard_repeat(175, 30);
        //TCOD_console_disable_keyboard_repeat();

        game->width = COLS;
        game->height = ROWS;

        game->left.w = COLS/4;
        game->left.h = (ROWS/3) * 2;
        game->left.x = 0;
        game->left.y = 0;
        game->left.c = TCOD_console_new(game->left.w, game->left.h);

        game->map.w = COLS/2;
        game->map.h = (ROWS/3) * 2;
        game->map.x = game->left.w + 1;
        game->map.y = 0;
        game->map.c = TCOD_console_new(game->map.w, game->map.h);

        game->right.w = COLS/4;
        game->right.h = (ROWS/3) * 2;
        game->right.x = game->map.x + game->map.w + 1;
        game->right.y = 0;
        game->right.c = TCOD_console_new(game->left.w, game->left.h);

        game->messages.w = COLS;
        game->messages.h = ROWS / 3;
        game->messages.x = 0;
        game->messages.y = game->left.h + 1;
        game->messages.c = TCOD_console_new(game->messages.w, game->messages.h);
        TCOD_console_set_default_foreground(game->messages.c, TCOD_white);
        TCOD_console_set_default_background(game->messages.c, TCOD_black);
        maxmess = game->messages.h - 2;
}

void shutdown_display()
{
        printf("Shutting down!\n");
}
// vim: fdm=syntax guifont=Terminus\ 8
