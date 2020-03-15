/**
  "Vegeta! What does the scouter say about his power level?"
*/

#ifndef _LEVEL_H_
#define _LEVEL_H_

#include "stdio.h"
#include "stdlib.h"
#include <SDL2/SDL.h>
#include "sdlike.h"


#define MAX_CELL_CAPACITY 255
#define MIN_CELL_CAPACITY 2


typedef enum ObjType {
  ERROR,
  GRASS,
  WALL
} ObjType;


/**
  Any object on the Level
  (grass, wall, hero, etc)
*/
typedef struct Object {
  ObjType type;
  Tile* tile;
} Object;


/**
  Another cell on the field.
  Contains objects on this place.
*/
typedef struct Cell {
  unsigned depth;
  unsigned capacity;
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
int place_level(
  Level* level,
  SDL_Surface* screen
);


int make_cell(
  char cell_symbol,
  TileStoreNode* tile_store,
  //
  Cell** cell
);


int make_object(
  ObjType type,
  TileStoreNode* tile_store,
  //
  Object** obj
);


int push_object_to_cell (
  Object* obj,
  Cell* cell
);

#endif // _LEVEL_H_

