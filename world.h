/*
 * Dark Shadows - The Roguelike
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef _WORLD_H
#define _WORLD_H

#include <stdbool.h>

#define CELL_NOTHING         0
#define CELL_WALL            1
#define CELL_FLOOR           2

// These might be used later
#define CELL_LAKE            100
#define CELL_MOUNTAIN        101

#define AREA_OUTSIDE                 0
#define AREA_COLLINWOOD_MAIN_FLOOR   1
#define AREA_COLLINWOOD_UPSTAIRS     2
#define AREA_COLLINWOOD_STUDY        3
#define AREA_COLLINWOOD_KITCHEN      4

// Exit types
#define ET_EXIT         1
#define ET_STAIRS_UP    2
#define ET_STAIRS_DOWN  3
#define ET_DOOR         4

#define YSIZE 1024
#define XSIZE 1024
#define DUNGEON_SIZE 200

#define cc(a,b) world->curlevel->c[a][b].color
#define cf(a,b) world->curlevel->c[a][b].flags
#define ci(a,b) world->curlevel->c[a][b].inventory
#define cm(a,b) world->curlevel->c[a][b].monster
#define ct(a,b) world->curlevel->c[a][b].type
#define cv(a,b) world->curlevel->c[a][b].visible

typedef struct {
        int location;
        int type;
        char c;
} exit_t;

typedef struct {                 // cell_t
        char       type;
        int        flags;
        short      desty, destx;       // for stairs and portals; destination y,x
        short      color;
        short      litcolor;
        bool       visible;
        signed int height;
        short      exitindex;
        monster_t *monster;
        inv_t     *inventory;
} cell_t;

struct levelstruct {
        short      xsize, ysize;
        short      level, type;
        int        zero;           // for defining the "zero" level of a heightmap (i.e. the mean value)
        int        lakelimit;
        cell_t   **c;
        monster_t *monsters;      // point to head of linked lists of monsters on this level
        obj_t     *objects;
        exit_t   exit[10];
};

struct room {
        int y1, x1, y2, x2, sx, sy;
};

typedef struct levelstruct level_t;
typedef cell_t** map_ptr;

typedef struct {
        level_t  *out;               // shall point to area[0]
        level_t  *area;
        level_t  *curlevel;          // needed?
        cell_t   **cmap;
} world_t;

typedef struct {
        int    y, x;
        cell_t **c;
        exit_t exit[10];
} areadef_t;

extern areadef_t areadef[50];

// CELLFLAGS

#define CF_HAS_STAIRS_DOWN   (1<<0)
#define CF_HAS_STAIRS_UP     (1<<1)
#define CF_LIT               (1<<2)
#define CF_VISITED           (1<<3)
#define CF_HAS_DOOR_OPEN     (1<<4)
#define CF_HAS_DOOR_CLOSED   (1<<5)
#define CF_HAS_DOOR_SECRET   (1<<6)
#define CF_IS_STARTING_POINT (1<<7)
#define CF_HAS_EXIT          (1<<8)


void generate_world();
void floodfill(level_t *l, int y, int x);
bool passable(level_t *l, int y, int x);
bool monster_passable(level_t *l, int y, int x);
void init_level(level_t *level);
void set_level_visited(level_t *l);
void pathfinder(level_t *l, int y, int x, int dy, int dx);

void addfloor(level_t *l, float y, float x);
void addwall(level_t *l, int y, int x);
void adddoor(level_t *l, int y, int x, bool secret);

void paint_room(level_t *l, int y, int x, int sy, int sx, int join_overlapping);
void paint_corridor(level_t *l, int y1, int x1, int y2, int x2);
void paint_corridor_vertical(level_t *l, int y1, int y2, int x);
void paint_corridor_horizontal(level_t *l, int y, int x1, int x2);

void generate_terrain(int visible);

extern char mapchars[];

#endif
