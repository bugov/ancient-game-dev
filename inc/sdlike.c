#include "sdlike.h"


int init_sdl(
  char* title,
  unsigned width,
  unsigned height,
  //
  SDL_Window** window,
  SDL_Surface** screen_surface
) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		return 1;
	}
	
	*window = SDL_CreateWindow(
	  title,
	  SDL_WINDOWPOS_UNDEFINED,
	  SDL_WINDOWPOS_UNDEFINED,
	  width,
	  height,
	  SDL_WINDOW_SHOWN
	);
	
	if (*window == NULL) {
		return 2;
	}
	
  *screen_surface = SDL_GetWindowSurface(*window);
  if (*screen_surface == NULL) {
    return 3;
  }
	
	return 0;
}


int destroy_sdl(SDL_Window** window, SDL_Surface** screen) {
	SDL_DestroyWindow(*window);
	*window = NULL;
	
	SDL_FreeSurface(*screen);
	*screen = NULL;

	SDL_Quit();
	return 0;
}


int load_image(
  char* path,
  //
  SDL_Surface** img
) {
  if ((*img = IMG_Load(path)) == NULL) {
    dp("Image could not load: `%s`. SDL_Error: %s\n", path, SDL_GetError());
    return 1;
  }
  
  return 0;
}


int place_surface_px(
  unsigned x, unsigned y,
  SDL_Surface* src, SDL_Surface* screen
) {
  SDL_Rect src_rect = {
    .x = 0,
    .y = 0,
    .w = CELL_WIDTH,
    .h = CELL_HEIGHT
  };
  SDL_Rect dst_rect = {
    .x = x,
    .y = y,
    .w = CELL_WIDTH,
    .h = CELL_HEIGHT,
  };
  
  if (SDL_BlitSurface(src, &src_rect, screen, &dst_rect)) {
    dp("Can't place a surface. SDL_Error: %s\n", SDL_GetError());
    return 1;
  }
  
  return 0;
}


int place_surface_pos(
  unsigned x, unsigned y,
  SDL_Surface* src, SDL_Surface* screen
) {
  return place_surface_px(
    x * CELL_WIDTH,
    y * CELL_HEIGHT,
    src,
    screen
  );
}


int make_tile(
  char* name,
  char* path,
  //
  Tile** return_tile
) {
  SDL_Surface* image;
  if(load_image(path, &image)) {
    dp("Failed on make tile: `%s`\n", name);
		return 1;
  }
  
  Tile* tile = (Tile*)malloc(sizeof(Tile));
  tile->name = name;
  tile->image = image;
  
  *return_tile = tile;
  
  return 0;
}


int add_to_tile_store(TileStoreNode* head, Tile* tile) {
  dp("Add tile to store: `%s`\n", tile->name);
  TileStoreNode* node = head;
  
  if (node->tile != NULL) {
    if (strcmp(node->tile->name, tile->name) == 0) {
      dp("Can't add tile (name already exists): `%s`\n", tile->name);
      return 1;
    }
  }

  while (node->next != NULL) {
    node = node->next;
    
    if (strcmp(node->tile->name, tile->name) == 0) {
      dp("Can't add tile (name already exists): `%s`\n", tile->name);
      return 1;
    }
  }
    
  TileStoreNode* new_node = (TileStoreNode*)malloc(sizeof(TileStoreNode));
  new_node->tile = tile;
  new_node->next = NULL;
  node->next = new_node;
  
  return 0;
}


int find_in_tile_store(
  TileStoreNode* head,
  char* name,
  //
  Tile** tile
) {
  dp("Find tile in store: `%s`\n", name);
  TileStoreNode* node = head;
  
  while (node->next != NULL) {
    node = node->next;
    
    if (strcmp(node->tile->name, name) == 0) {
      *tile = node->tile;
      return 0;
    }
  }
  
  return 1;
}

