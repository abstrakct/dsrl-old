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

#include "npc-names.h"
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

bool blocks_light(level_t *l, int y, int x)
{
    if(hasbit(l->c[y][x].flags, CF_HAS_DOOR_CLOSED)) {
        return true;
    }
    if(hasbit(l->c[y][x].flags, CF_HAS_DOOR_OPEN)) {
        return false;
    }

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
    TCOD_console_clear(game->messages.c);
    currmess++;
    for(i = maxmess-1; i > 0; i--) {
        TCOD_console_set_default_foreground(game->messages.c, messages[i].color);
        TCOD_console_print(game->messages.c, 1, i, "%s", messages[i].text);
    }
    TCOD_console_blit(game->messages.c, 0, 0, game->messages.w, game->messages.h, NULL, game->messages.x, game->messages.y, 1.0, 1.0);
    TCOD_console_flush();

    //fprintf(messagefile, "%d - %s\n", game->turn, messages[currmess-1].text);
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

void donewfov(actor_t *a, level_t *l)
{
    TCOD_map_compute_fov(l->map, a->x, a->y, a->viewradius, true, FOV_SHADOW);
}

void newfov_initmap(level_t *l)
{
    int x, y;
    bool trans, walk;

    for(x = 1; x < l->xsize; x++) {
        for(y = 1; y < l->ysize; y++) {
            if(blocks_light(l, y, x))
                trans = false;
            else
                trans = true;

            /*switch(l->c[y][x].type) {
              case CELL_NOTHING:
              case CELL_WALL:
              trans = false;
              walk = false;
              break;
              case CELL_FLOOR:
              walk = true;
              break;
              }*/

            if(passable(l, y, x))
                walk = true;
            else
                walk = false;


            TCOD_map_set_properties(l->map, x, y, trans, walk);
        }
    }
}

void fov_updatemap(void *level)
{
    int x, y;
    bool trans, walk;
    level_t *l;

    l = (level_t *)level;

    for(x = 1; x < l->xsize; x++) {
        for(y = 1; y < l->ysize; y++) {
            if(blocks_light(l, y, x))
                trans = false;
            else
                trans = true;

            if(passable(l, y, x))
                walk = true;
            else
                walk = false;


            TCOD_map_set_properties(l->map, x, y, trans, walk);
        }
    }
}

void fov_initmap(void *level)
{
    level_t *l;

    l = (level_t *)level;

    l->map = TCOD_map_new(l->xsize, l->ysize);
    fov_updatemap(l);
}

void draw_left()
{
    int i;
    TCOD_color_t color;

    i = 0;

    //TCOD_console_set_alignment(game->left.c, TCOD_CENTER);
    TCOD_console_set_default_foreground(game->left.c, TCOD_light_blue);
    TCOD_console_print(game->left.c, (game->left.w/2)-(strlen(player->name)/2), i+1, "%c %s %c", 236, player->name, 236);
    TCOD_console_set_default_foreground(game->left.c, TCOD_white);
    TCOD_console_print(game->left.c, (game->left.w/2)-9, i+2, "%s %d, %d - %02d:%02d", monthstring[game->t.month], game->t.day, game->t.year, game->t.hour, game->t.minute);

    i++;
    TCOD_console_print(game->left.c, 1, i+3, "Weapon: %s", player->weapon ? player->weapon->fullname : "bare hands");
    //TCOD_console_print(game->left.c, 1, i+4, "viewradius: %d", player->viewradius);

    /* Hitpoints */
    if(player->hp >= (player->maxhp/4*3))
        color = TCOD_green;
    else if(player->hp >= (player->maxhp/4) && player->hp < (player->maxhp/4*3))
        color = TCOD_yellow;
    else if(player->hp < (player->maxhp/4))
        color = TCOD_red;

    TCOD_console_print(game->left.c, 1, i+4, "HP:");
    TCOD_console_set_default_foreground(game->left.c, color);
    TCOD_console_print(game->left.c, 5, i+4, "%d/%d (%.1f%%)", player->hp, player->maxhp, ((float)(100/(float)player->maxhp) * (float)player->hp));

    TCOD_console_set_default_foreground(game->left.c, TCOD_white);
    TCOD_console_print(game->left.c, 1, i+6,  "AC: %d", player->ac);
    TCOD_console_print(game->left.c, 1, i+7,  "STR:   %d", player->attr.str);
    TCOD_console_print(game->left.c, 1, i+8,  "DEX:   %d", player->attr.dex);
    TCOD_console_print(game->left.c, 1, i+9,  "PHY:   %d", player->attr.phy);
    TCOD_console_print(game->left.c, 1, i+10, "INT:   %d", player->attr.intl);
    TCOD_console_print(game->left.c, 1, i+11, "WIS:   %d", player->attr.wis);
    TCOD_console_print(game->left.c, 1, i+12, "CHA:   %d", player->attr.cha);
    TCOD_console_print(game->left.c, 1, i+13, "XP:    %d", player->xp);
    TCOD_console_print(game->left.c, 1, i+14, "Level: %d", player->level);
    TCOD_console_print(game->left.c, 1, i+15, "Tick:  %d", game->tick);

    //TCOD_console_print(game->left.c, 1, i+9, 1, "Dungeon level: %d (out of %d)", game->currentlevel, game->createdareas);
    //mvwprintw(wleft, 3, 1, "y,x     %d,%d", ply, plx);
    //mvwprintw(wleft, 4, 1, "(py,px) (%d,%d)", ppy, ppx);
}

void draw_right()
{
    obj_t *o;
    int i, j;

    TCOD_console_set_default_foreground(game->right.c, TCOD_light_blue);
    TCOD_console_print(game->right.c, (game->right.w/2)-7, 1, "%c INVENTORY %c", 228+8, 228+8);

    TCOD_console_set_default_foreground(game->right.c, TCOD_yellow);
    TCOD_console_print(game->right.c, 1, 3, "Gold: %d", player->inventory->gold);


    TCOD_console_set_default_foreground(game->right.c, TCOD_white);

    i = 4;
    for(j = 0; j < 52; j++) {
        if(player->inventory->object[j]) {
            //o = get_object_from_letter(slot_to_letter(j), player->inventory);
            o = player->inventory->object[j];
            if(is_worn(o)) {
                if(is_pair(o))
                    TCOD_console_print(game->right.c, 1, i, "%c   a pair of %s %s", slot_to_letter(j), o->fullname, is_bracelet(o) ? (o == pw_leftbracelet ? "[<]" : "[>]") : "\0");
                else
                    TCOD_console_print(game->right.c, 1, i, "%c   %s %s", slot_to_letter(j), a_an(o->fullname), is_bracelet(o) ? (o == pw_leftbracelet ? "[<]" : "[>]") : "\0");
                TCOD_console_put_char_ex(game->right.c, 3, i, '*', TCOD_light_green, TCOD_black);
            } else {
                if(is_pair(o))
                    TCOD_console_print(game->right.c, 1, i, "%c   a pair of %s", slot_to_letter(j), o->fullname);
                else
                    TCOD_console_print(game->right.c, 1, i, "%c   %s", slot_to_letter(j), a_an(o->fullname));
                TCOD_console_put_char_ex(game->right.c, 3, i, '-', TCOD_white, TCOD_black);
            }
            i++;
        }
    }
}

void draw_map(level_t *level)
{
    int i,j, slot;
    int dx, dy;  // coordinates on screen!
    TCOD_color_t color;
    TCOD_color_t fore, back;

    //FOV(player, level);
    //if(game->context == CONTEXT_INSIDE)
    //        FOVlight(player, level);     // only necessary inside


    donewfov(player, level);

    /*
     * in this function, (j,i) are the coordinates on the map,
     * dx,dy = coordinates on screen.
     * so, player->py/px describes the upper left corner of the map
     */
    for(i = ppx, dx = 1; i < (ppx + game->map.w - 2); i++, dx++) {
        for(j = ppy, dy = 1; j < (ppy + game->map.h - 2); j++, dy++) {
            if(j < level->ysize && i < level->xsize) {
                if(TCOD_map_is_in_fov(level->map, i, j))
                    setbit(level->c[j][i].flags, CF_VISITED);

                if(hasbit(level->c[j][i].flags, CF_VISITED)) {
                    if(TCOD_map_is_in_fov(level->map, i, j)) {
                        if(ct(j, i) == CELL_WALL)
                            color = TCOD_dark_red;
                        else if(ct(j, i) == CELL_FLOOR)
                            color = TCOD_gray;
                    } else {
                        color = TCOD_darkest_gray;
                    }
                }

                if(hasbit(level->c[j][i].flags, CF_VISITED)) {
                    dsmapaddch(dy, dx, color, level->c[j][i].backcolor, mapchars[(int) level->c[j][i].type]);

                    if(hasbit(level->c[j][i].flags, CF_HAS_FURNITURE)) {
                        if(hasbit(level->c[j][i].flags, CF_HASF_TABLE))
                            dsmapaddch(dy, dx, TCOD_darker_sepia, level->c[j][i].backcolor, 'T');
                        if(hasbit(level->c[j][i].flags, CF_HASF_CHAIR))
                            dsmapaddch(dy, dx, TCOD_crimson, level->c[j][i].backcolor, 'h');
                        if(hasbit(level->c[j][i].flags, CF_HASF_FIRE))
                            dsmapaddch(dy, dx, TCOD_flame, level->c[j][i].backcolor, 21);
                    }


                    if(level->c[j][i].inventory) {
                        if(level->c[j][i].inventory->gold > 0) {
                            dsmapaddch(dy, dx, TCOD_gold, level->c[j][i].backcolor, objchars[OT_GOLD]);
                        } else {                                                         // TODO ADD OBJECT COLORS!!!
                            slot = get_first_used_slot(level->c[j][i].inventory);
                            if(level->c[j][i].inventory->num_used > 0 && slot >= 0 && level->c[j][i].inventory->object[slot]) {
                                fore = level->c[j][i].inventory->object[slot]->fore;
                                back = level->c[j][i].inventory->object[slot]->back;
                                dsmapaddch(dy, dx, fore, back, objchars[level->c[j][i].inventory->object[slot]->type]);
                            }
                        }
                    }

                    if(hasbit(level->c[j][i].flags, CF_HAS_DOOR_CLOSED)) {
                        //dsprintf("closed door at %d,%d", dx, dy);
                        dsmapaddch(dy, dx, color, level->c[j][i].backcolor, '+');
                    }
                    if(hasbit(level->c[j][i].flags, CF_HAS_DOOR_OPEN)) {
                        //dsprintf("open door at %d,%d", dx, dy);
                        dsmapaddch(dy, dx, color, level->c[j][i].backcolor, '\'');
                    }

                    if(hasbit(level->c[j][i].flags, CF_HAS_STAIRS_DOWN))
                        dsmapaddch(dy, dx, TCOD_white, level->c[j][i].backcolor, '>');
                    if(hasbit(level->c[j][i].flags, CF_HAS_STAIRS_UP))
                        dsmapaddch(dy, dx, TCOD_white, level->c[j][i].backcolor, '<');
                    if(hasbit(level->c[j][i].flags, CF_HAS_EXIT)) {
                        int index;
                        index = level->c[j][i].exitindex;
                        if(level->exit[index].type == ET_EXIT)
                            dsmapaddch(dy, dx, TCOD_white, level->c[j][i].backcolor, '^');
                        if(level->exit[index].type == ET_STAIRS_UP)
                            dsmapaddch(dy, dx, TCOD_white, level->c[j][i].backcolor, '|');
                        if(level->exit[index].type == ET_STAIRS_DOWN)
                            dsmapaddch(dy, dx, TCOD_white, level->c[j][i].backcolor, '>');
                        if(level->exit[index].type == ET_DOOR)
                            dsmapaddch(dy, dx, TCOD_white, level->c[j][i].backcolor, '+');
                    } 

                    if(TCOD_map_is_in_fov(level->map, i, j) && level->c[j][i].monster /*&& actor_in_lineofsight(player, level->c[j][i].monster)*/)
                        dsmapaddch(dy, dx, TCOD_red, level->c[j][i].backcolor, (char) level->c[j][i].monster->c);

                    if(TCOD_map_is_in_fov(level->map, i, j) && level->c[j][i].npc)
                        dsmapaddch(dy, dx, TCOD_green, TCOD_black, '@');

                    if(j == ply && i == plx)
                        dsmapaddch(dy, dx, TCOD_blue, level->c[j][i].backcolor, '@');
                }
            }
        }
    }
}

void dsmapaddch(int y, int x, TCOD_color_t color, TCOD_color_t backcolor, char c)
{
    TCOD_console_put_char_ex(game->map.c, x, y, c, color, backcolor);
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
    TCOD_console_clear(game->map.c);
    TCOD_console_clear(game->left.c);
    //TCOD_console_clear(game->right.c);

    //TCOD_console_rect(game->map.c, game->map.x, game->map.y, game->map.w, game->map.h, true, TCOD_BKGND_NONE);
    /*TCOD_console_print_frame(game->map.c, 0, 0, game->map.w, game->map.h, true, TCOD_BKGND_NONE, "Map");
      TCOD_console_print_frame(game->left.c, 0, 0, game->left.w, game->left.h, true, TCOD_BKGND_NONE, "You");
      TCOD_console_print_frame(game->right.c, 0, 0, game->right.w - 2, game->right.h, true, TCOD_BKGND_NONE, "Inventory");*/

    draw_map(world->curlevel);
    draw_left();
    //draw_right();

    TCOD_console_blit(game->map.c, 0, 0, game->map.w, game->map.h, NULL, game->map.x, game->map.y, 1.0, 1.0);
    TCOD_console_blit(game->messages.c, 0, 0, game->messages.w, game->messages.h, NULL, game->messages.x, game->messages.y, 1.0, 1.0);
    TCOD_console_blit(game->left.c, 0, 0, game->left.w, game->left.h, NULL, game->left.x, game->left.y, 1.0, 1.0);
    //TCOD_console_blit(game->right.c, 0, 0, game->right.w, game->right.h, NULL, game->right.x, game->right.y, 1.0, 1.0);

    TCOD_console_flush();
}

void draw_wstat()
{
}

TCOD_key_t dsgetch()
{
    TCOD_key_t key;

    TCOD_console_flush();
    key = TCOD_console_wait_for_keypress(false);
    //key = TCOD_console_check_for_keypress(TCOD_KEY_PRESSED);

    if(key.shift && key.c >= 'a' && key.c <= 'z')
        key.c += 'A' - 'a';
    if(key.shift && key.c == '<')
        key.c = '>';

    return key;
}

bool ds_checkforkeypress()
{
    TCOD_key_t key;

    //TCOD_console_flush();
    key = TCOD_console_check_for_keypress(TCOD_KEY_PRESSED);
    if(key.vk == TCODK_NONE)
        return false;
    else
        return true;
}

void init_display()
{ 
    /* font selection code is stolen from brogue! */
    char font[60];
    //int fontsize = -1;

    //int screenwidth = 1024;
    //int screenheight = 600;
    //int fontwidths[13] = {112, 128, 144, 160, 176, 192, 208, 224, 240, 256, 272, 288, 304}; // widths of the font graphics (divide by 16 to get individual character width)
    //int fontheights[13] = {176, 208, 240, 272, 304, 336, 368, 400, 432, 464, 496, 528, 528}; // heights of the font graphics (divide by 16 to get individual character height)

    //TCOD_sys_get_current_resolution(&screenwidth, &screenheight);
    //screenwidth = 1024;
    //screenheight = 600;

    // adjust for title bars and whatever -- very approximate, but better than the alternative
    /*screenwidth -= 6;
      screenheight -= 48;*/

    /*
    dsconfig.rows = ROWS;
    dsconfig.cols = COLS;

    if (fontsize < 1 || fontsize > 13) {
        for (fontsize = 13; fontsize > 1 && (fontwidths[fontsize - 1] * dsconfig.cols / 16 >= screenwidth || fontheights[fontsize - 1] * dsconfig.rows / 16 >= screenheight); fontsize--);
    }

    if(screenwidth <= 1024) {
        sprintf(font, "fonts/ds.png");
        //dsconfig.rows *= 2;
    } else {
        sprintf(font, "fonts/df.png");
        sprintf(font, "fonts/terminal8x14_gs_ro.png");
        TCOD_console_set_custom_font(font, TCOD_FONT_TYPE_GREYSCALE | TCOD_FONT_LAYOUT_ASCII_INROW, 16, 16);
        dsconfig.rows = screenheight / 18;
        dsconfig.cols = screenwidth  / 18;
    }
*/
    // override evertyhing
    sprintf(font, "fonts/terminal8x14_gs_ro.png");
    TCOD_console_set_custom_font(font, TCOD_FONT_TYPE_GREYSCALE | TCOD_FONT_LAYOUT_ASCII_INROW, 16, 16);
    dsconfig.rows = 40;
    dsconfig.cols = 125;

    //sprintf(font, "fonts/font-%i.png", fontsize);

    //TCOD_console_set_custom_font(font, /*TCOD_FONT_TYPE_GREYSCALE |*/ TCOD_FONT_LAYOUT_ASCII_INROW, 16, 16);

    TCOD_console_init_root(dsconfig.cols, dsconfig.rows, GAME_NAME, false, TCOD_RENDERER_SDL);
    TCOD_console_map_ascii_codes_to_font(0, 255, 0, 0);
    TCOD_console_set_keyboard_repeat(350, 70);

    game->width = dsconfig.cols;
    game->height = dsconfig.rows;

    game->left.w = dsconfig.cols/4;
    game->left.h = (dsconfig.rows/3) * 2;
    game->left.x = 0;
    game->left.y = 0;
    game->left.c = TCOD_console_new(game->left.w, game->left.h);

    game->map.w = dsconfig.cols/4*3;
    game->map.h = (dsconfig.rows/3) * 2;
    game->map.x = game->left.w + 1;
    game->map.y = 0;
    game->map.c = TCOD_console_new(game->map.w, game->map.h);

    /*game->right.w = dsconfig.cols/4;
      game->right.h = (dsconfig.rows/3) * 2;
      game->right.x = game->map.x + game->map.w + 1;
      game->right.y = 0;
      game->right.c = TCOD_console_new(game->left.w, game->left.h);*/

    game->messages.w = dsconfig.cols;
    game->messages.h = dsconfig.rows / 3;
    game->messages.x = 0;
    game->messages.y = game->left.h + 1;
    game->messages.c = TCOD_console_new(game->messages.w, game->messages.h);
    TCOD_console_set_default_foreground(game->messages.c, TCOD_white);
    TCOD_console_set_default_background(game->messages.c, TCOD_black);
    maxmess = game->messages.h - 2;

    TCOD_console_set_window_title(GAME_NAME);
}

void shutdown_display()
{
    printf("Shutting down!\n");
}

// Messageboxes etc.

void gtmsgbox(char *header, char *message)
{
    TCOD_console_t c;
    int w, h, x, y;

    w = game->width / 4;
    w += 4;
    h = TCOD_console_get_height_rect(NULL, 1, 1, w-4, game->height, message);
    h += 2;

    c = TCOD_console_new(w, h);

    TCOD_console_set_default_background(c, TCOD_black);
    TCOD_console_set_default_foreground(c, TCOD_white);
    //TCOD_console_clear(c);

    //TCOD_console_print_rect(c, 0, 0, w, h, header);
    // write msg
    TCOD_console_print_frame(c, 0, 0, w, h, true, TCOD_BKGND_NONE, header);
    TCOD_console_print_rect(c, 1, 1, w-4, game->height, message);
    //TCOD_console_print_rect_ex(c, 1, 1, w, h, TCOD_BKGND_NONE, TCOD_LEFT, header);

    x = ((game->map.w + game->left.w + game->right.w) / 2) - (w / 2);
    y = ((game->map.h + game->messages.h) / 2) - (h / 2);

    TCOD_console_blit(c, 0, 0, w, h, NULL, x, y, 1.0, 1.0);
    TCOD_console_flush();

    while(!ds_checkforkeypress());
    ds_checkforkeypress();

    TCOD_console_clear(c);
    TCOD_console_blit(c, 0, 0, w, h, NULL, x, y, 1.0, 1.0);
    TCOD_console_flush();
}

void show_inventory(void *inv_p)
{
    TCOD_console_t c;
    int w, h, x, y;
    obj_t *o;
    inv_t *inv;
    int i, j;

    inv = (inv_t *) inv_p;

    w = game->width / 4;
    w += 4;
    h = get_num_used_slots(inv); //TCOD_console_get_height_rect(NULL, 1, 1, w-4, game->height, message);
    h += 2;

    c = TCOD_console_new(w, h);

    TCOD_console_set_default_background(c, TCOD_black);
    TCOD_console_set_default_foreground(c, TCOD_white);

    TCOD_console_print_frame(c, 0, 0, w, h, true, TCOD_BKGND_NONE, "* INVENTORY *");

    TCOD_console_set_default_foreground(game->right.c, TCOD_white);

    i = 1;
    for(j = 0; j < 52; j++) {
        if(inv->object[j]) {
            //TODO:SIMPLIFY
            o = inv->object[j];
            if(is_worn(o)) {
                TCOD_console_print(c, 1, i, "%c", slot_to_letter(j));
                TCOD_console_put_char_ex(c, 3, i, '*', TCOD_light_green, TCOD_black);
                TCOD_console_set_default_foreground(c, o->fore);
                TCOD_console_set_default_background(c, o->back);
                TCOD_console_print(c, 5, i, "%s %s", a_an(pair(o)), is_bracelet(o) ? (o == pw_leftbracelet ? "[<]" : "[>]") : "\0");
                TCOD_console_set_default_foreground(c, TCOD_white);
                TCOD_console_set_default_background(c, TCOD_black);
            } else {
                TCOD_console_print(c, 1, i, "%c", slot_to_letter(j));
                TCOD_console_put_char_ex(c, 3, i, '-', TCOD_white, TCOD_black);
                TCOD_console_set_default_foreground(c, o->fore);
                TCOD_console_set_default_background(c, o->back);
                if(o->quantity <= 1)
                    TCOD_console_print(c, 5, i, "%s", a_an(pair(o)));
                else if(o->quantity > 1) 
                    TCOD_console_print(c, 5, i, "%d %s", o->quantity, plural(o));
                TCOD_console_set_default_foreground(c, TCOD_white);
                TCOD_console_set_default_background(c, TCOD_black);
            }
            i++;
        }
    }

    x = ((game->map.w + game->left.w + game->right.w) / 2) - (w / 2);
    y = ((game->map.h + game->messages.h) / 2) - (h / 2);

    TCOD_console_blit(c, 0, 0, w, h, NULL, x, y, 1.0, 1.0);
    TCOD_console_flush();

    while(!ds_checkforkeypress());
    ds_checkforkeypress();

    TCOD_console_clear(c);
    TCOD_console_blit(c, 0, 0, w, h, NULL, x, y, 1.0, 1.0);
    TCOD_console_flush();

}

void show_player_inventory()
{
    show_inventory(player->inventory);
}


// vim: fdm=syntax 
