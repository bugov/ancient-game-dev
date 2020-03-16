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
#define WALK_FRAMES 8
#define WALK_STEP_PX 16


typedef enum ObjType {
  OBJECT_ERROR,
  OBJECT_GRASS,
  OBJECT_WALL,
  OBJECT_HERO,
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
  
  char can_walk;
  char passable;
  char walkable;
  
  char walk_frame;
  ObjDirection direction;
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


/**
  Read text file with level definition
  and create a level.
*/
int make_level_from_file(
  char* path,
  TileStoreNode* tile_store,
  //
  Level** level
);


/**
  Debug function. Prints level to stdout.
*/
void print_level(Level* level);


/**
  Put the level to the screen.
*/
int place_stay_level(
  Level* level,
  SDL_Surface* screen
);


int place_walk_object(
  Level* level,
  SDL_Surface* screen,
  Object* obj
);


int make_cell(
  char cell_symbol,
  TileStoreNode* tile_store,
  unsigned x_pos,
  unsigned y_pos,
  //
  Cell** cell
);


int is_cell_passable(Cell* cell);


int set_walk_object_to_direction(
  Object* obj,
  ObjDirection direction,
  Level* level
);


int animate_walk_frame(Object* obj);


int make_object(
  ObjType type,
  TileStoreNode* tile_store,
  unsigned x_pos,
  unsigned y_pos,
  //
  Object** obj
);


void attack_object(Object* src, Object* dst);

int push_object_to_cell (
  Object* obj,
  Cell* cell
);


int remove_object_from_cell (
  Object* obj,
  Cell* cell
);


int swap_object_between_cells(
  Object* obj,
  Cell* src,
  Cell* dst
);

#endif // _LEVEL_H_

