/**
  "She's dead. Wrapped in plastic."
*/
#ifndef _SDLIKE_H_
#define _SDLIKE_H_

#include "stdio.h"
#include "stdlib.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "const.h"


#ifdef DEBUG
#define dp(...) do { fprintf( stderr, __VA_ARGS__ ); } while (0)
#else
#define dp(...) do { } while (0)
#endif


/**
  Initialize SDL window and screen surface.
*/
int init_sdl(
  char* title,
  unsigned width,
  unsigned height,
  //
  SDL_Window** window,
  SDL_Surface** screen_surface
);


/**
  Destroy/free SDL window and screen surface.
*/
int destroy_sdl(
  SDL_Window** window,
  SDL_Surface** screen
);


/**
  Load surface from image file.
*/
int load_image(
  char* path,
  //
  SDL_Surface** img
);


int place_surface_part_px(
  unsigned from_x,
  unsigned from_y,
  unsigned to_x,
  unsigned to_y,
  SDL_Surface* src,
  SDL_Surface* screen
);


int place_surface_part_pos(
  unsigned from_x,
  unsigned from_y,
  unsigned to_x,
  unsigned to_y,
  SDL_Surface* src,
  SDL_Surface* screen
);


/**
  Copy image data into screen surface.
  x, y - position on screen in PIXELS.
  w, h - CELL_*
*/
int place_surface_px(
  unsigned x,
  unsigned y,
  SDL_Surface* src,
  SDL_Surface* screen
);


/**
  Copy image data into screen surface.
  x, y - position on screen in CELLS.
  w, h - CELL_*
*/
int place_surface_pos(
  unsigned x, unsigned y,
  SDL_Surface* src, SDL_Surface* screen
);


typedef struct Tile {
  char* name;
  SDL_Surface* image;
} Tile;


typedef struct TileStoreNode {
  struct TileStoreNode* next;
  Tile* tile;
} TileStoreNode;


int add_to_tile_store(
  TileStoreNode* head,
  Tile* tile
);


int find_in_tile_store(
  TileStoreNode* head,
  char* name,
  //
  Tile** tile
);


int make_tile(
  char* name,
  char* path,
  //
  Tile** return_tile
);


void make_cursor(
  const char* image[],
  SDL_Cursor** cursor
);

#endif // _SDLIKE_H_

