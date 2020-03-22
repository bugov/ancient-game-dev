/**
  "She's dead. Wrapped in plastic."
*/
#ifndef _SDLIKE_H_
#define _SDLIKE_H_

#include "stdio.h"
#include "stdlib.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "common.h"


/**
  Initialize SDL window and screen surface.
*/
int init_sdl(
  char* title,
  unsigned width,
  unsigned height,
  //
  SDL_Window** window,
  SDL_Renderer** renderer
);


/**
  Destroy/free SDL window and screen surface.
*/
int destroy_sdl(
  SDL_Window** window,
  SDL_Renderer** renderer
);


/**
  Load surface from image file.
*/
int load_image(
  char* path,
  SDL_Renderer* renderer,
  //
  SDL_Texture** img
);


int render_surface_part_px(
  unsigned from_x,
  unsigned from_y,
  unsigned to_x,
  unsigned to_y,
  SDL_Texture* src,
  SDL_Renderer* renderer
);


int render_surface_part_pos(
  unsigned from_x,
  unsigned from_y,
  unsigned to_x,
  unsigned to_y,
  SDL_Texture* src,
  SDL_Renderer* renderer
);


/**
  Copy image data into screen surface.
  x, y - position on screen in PIXELS.
  w, h - CELL_*
*/
int render_surface_px(
  unsigned x,
  unsigned y,
  SDL_Texture* src,
  SDL_Renderer* renderer
);


/**
  Copy image data into screen surface.
  x, y - position on screen in CELLS.
  w, h - CELL_*
*/
int render_surface_pos(
  unsigned x,
  unsigned y,
  SDL_Texture* src,
  SDL_Renderer* renderer
);


typedef struct Tile {
  char* name;
  SDL_Texture* image;
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
  SDL_Renderer* renderer,
  //
  Tile** return_tile
);


int set_viewport(
  int x,
  int y,
  int w,
  int h,
  SDL_Renderer* renderer
);

#endif // _SDLIKE_H_

