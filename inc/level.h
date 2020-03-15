/**
  "Vegeta! What does the scouter say about his power level?"
*/

#ifndef _LEVEL_H_
#define _LEVEL_H_

#include "stdio.h"
#include "stdlib.h"
#include <SDL2/SDL.h>
#include "sdlike.h"


/**
  Another cell on the field.
  Contains objects on this place.
*/
typedef struct Cell {
  char* type;
  SDL_Surface* image;
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

#endif // _LEVEL_H_

