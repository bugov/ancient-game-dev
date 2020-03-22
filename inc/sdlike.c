#include "sdlike.h"


int init_sdl(
  char* title,
  unsigned width,
  unsigned height,
  //
  SDL_Window** window,
  SDL_Renderer** renderer
) {
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		return 1;
	}
	
	*window = SDL_CreateWindow(
	  title,
	  SDL_WINDOWPOS_UNDEFINED,
	  SDL_WINDOWPOS_UNDEFINED,
	  width,
	  height,
	  SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
	);
	
	if (*window == NULL) {
		return 2;
	}
	
  *renderer = SDL_CreateRenderer(
    *window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
  );
  
  if (*renderer == NULL) {
    return 3;
  }
	
	return 0;
}


int destroy_sdl(SDL_Window** window, SDL_Renderer** renderer) {
	SDL_DestroyRenderer(*renderer);
	*renderer = NULL;
	SDL_DestroyWindow(*window);
	*window = NULL;
	SDL_Quit();
	return 0;
}


int load_image(
  char* path,
  SDL_Renderer* renderer,
  //
  SDL_Texture** img
) {
  SDL_Surface* img_surf = IMG_Load(path);
  if (img_surf == NULL) {
    dp("Image could not load: `%s`. SDL_Error: %s\n", path, SDL_GetError());
    return 1;
  }
  
  *img = SDL_CreateTextureFromSurface(renderer, img_surf);
  SDL_FreeSurface(img_surf);
  
  if (*img == NULL) {
    dp("Can't load `%s`: `%s`.\n", path, SDL_GetError());
  }
  
  return 0;
}

int render_surface_part_px(
  unsigned from_x,
  unsigned from_y,
  unsigned to_x,
  unsigned to_y,
  SDL_Texture* src,
  SDL_Renderer* renderer
) {
  SDL_Rect src_rect = {
    .x = from_x,
    .y = from_y,
    .w = CELL_WIDTH,
    .h = CELL_HEIGHT
  };
  SDL_Rect dst_rect = {
    .x = to_x,
    .y = to_y,
    .w = CELL_WIDTH,
    .h = CELL_HEIGHT,
  };

  if (SDL_RenderCopy(renderer, src, &src_rect, &dst_rect)) {
    dp("Can't render a texture. SDL_Error: %s\n", SDL_GetError());
    return 1;
  }
  
  return 0;
}


int render_surface_part_pos(
  unsigned from_x,
  unsigned from_y,
  unsigned to_x,
  unsigned to_y,
  SDL_Texture* src,
  SDL_Renderer* renderer
) {
  return render_surface_part_px(
    from_x * CELL_WIDTH,
    from_y * CELL_HEIGHT,
    to_x * CELL_WIDTH,
    to_y * CELL_HEIGHT,
    src,
    renderer
  );
}


int render_surface_px(
  unsigned x,
  unsigned y,
  SDL_Texture* src,
  SDL_Renderer* renderer
) {
  return render_surface_part_px(0, 0, x, y, src, renderer);
}


int render_surface_pos(
  unsigned x,
  unsigned y,
  SDL_Texture* src,
  SDL_Renderer* renderer
) {
  return render_surface_px(
    x * CELL_WIDTH,
    y * CELL_HEIGHT,
    src,
    renderer
  );
}


int make_tile(
  char* name,
  char* path,
  SDL_Renderer* renderer,
  //
  Tile** return_tile
) {
  SDL_Texture* image;
  if(load_image(path, renderer, &image)) {
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
  //dp("Find tile in store: `%s`\n", name);
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


int set_viewport(int x, int y, int w, int h, SDL_Renderer* renderer) {
  SDL_Rect rect = {
    .x = x,
    .y = y,
    .w = w,
    .h = h
  };
  
  if (SDL_RenderSetViewport(renderer, &rect) != 0) {
    dp("Error set viewport: %s\n", SDL_GetError());
    return 1;
  }
  return 0;
}

