/**
  "Vegeta! What does the scouter say about his power level?"
*/

#ifndef _LEVEL_H_
#define _LEVEL_H_

#include "stdio.h"
#include "stdlib.h"
#include <SDL2/SDL.h>
#include "sdlike.h"


#define MAX_CELL_CAPACITY 256
#define MIN_CELL_CAPACITY 2
#define WALK_FRAMES 6
#define WALK_STEP_PX 16


typedef struct {
  char* text;
  SDL_Texture* image;
  unsigned w;
  unsigned h;
} Message;


typedef enum ObjType {
  OBJECT_ERROR,
  OBJECT_GRASS,
  OBJECT_WALL,
  OBJECT_WALL_TOP,
  OBJECT_DOOR,
  OBJECT_HERO,
  OBJECT_HUMAN,
  OBJECT_BARREL
} ObjType;


typedef enum ObjState {
  STATE_ERROR,
  STATE_STAY,
  STATE_WALK,
} ObjState;


typedef enum ObjDirection {
  UP, LEFT, DOWN, RIGHT
} ObjDirection;


/**
  Any object on the Level
  (grass, wall, hero, etc)
*/
typedef struct Object {
  ObjType type;
  ObjState state;
  Tile* tile;
  Tile* walk_tile;
  
  int hp;
  unsigned base_attack;
  
  unsigned x_pos;
  unsigned y_pos;
  unsigned x_px;
  unsigned y_px;
  
  char can_walk;
  char passable;
  char walkable;
  char talkable;
  
  char animation_frame;
  
  ObjDirection direction;
  
  Message** messages;
} Object;


/**
  Another cell on the field.
  Contains objects on this place.
*/
typedef struct Cell {
  unsigned depth;
  unsigned capacity;
  
  unsigned x_pos;
  unsigned y_pos;
  
  char changed;
  
  Object** objects;
} Cell;


/**
  World's level. Gaming area. 
  Field "cells" is two dimentional array of Cell.
*/
typedef struct Level {
  unsigned w;
  unsigned h;
  Cell*** cells;
} Level;


typedef enum GameMode {
  MODE_NORMAL,
  MODE_ATTACK,
  MODE_TALK
} GameMode;


typedef struct Context {
  GameMode mode;
  TileStoreNode* tile_store;
  Level* level;
  Object* hero;
  SDL_Renderer* renderer;
  unsigned window_width;
  unsigned window_height;
  char is_running;
  unsigned is_busy;
} Context;


int make_object(
  Context* ctx,
  ObjType type,
  unsigned x_pos,
  unsigned y_pos,
  //
  Object** obj_ptr
);


int set_walk_object_to_direction(
  Context* ctx,
  Object* obj,
  ObjDirection direction
);


int update_walk_frame(Context* ctx, Object* obj);


int make_cell_from_char(
  Context* ctx,
  char cell_symbol,
  unsigned x_pos,
  unsigned y_pos,
  //
  Cell** cell_ptr
);


int push_object_to_cell (
  Object* obj,
  Cell* cell
);


int remove_object_from_cell (
  Object* obj,
  Cell* cell
);


int is_cell_passable(Cell* cell);


int make_level_from_file(
  Context* ctx,
  char* path,
  //
  Level** level_ptr
);


void attack_object(Object* src, Object* dst);


int render_level(Context* ctx);


int swap_object_between_cells(
  Object* obj,
  Cell* src,
  Cell* dst
);

#endif // _LEVEL_H_

