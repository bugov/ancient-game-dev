#include <assert.h>
#include "level.h"
#include "sdlike.h"


int load_tiles(TileStoreNode* tile_store) {
  int success = 0;
  Tile* tile = NULL;
  
  success |= make_tile("grass", "./tiles/grass.png", &tile);
  add_to_tile_store(tile_store, tile);
  
  success |= make_tile("wall", "./tiles/wall.png", &tile);
  add_to_tile_store(tile_store, tile);
  
  success |= make_tile("hero_walk", "./tiles/hero_walk.png", &tile);
  add_to_tile_store(tile_store, tile);
  
  success |= make_tile("hero", "./tiles/hero.png", &tile);
  add_to_tile_store(tile_store, tile);
  
  return success;
}


int main(void) {
  SDL_Window* window = NULL;
  SDL_Surface* screen_surface = NULL;
  
  if (init_sdl("Hello SDL", 640, 640, &window, &screen_surface)) {
    dp("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    goto cleanup;
  }
  
  TileStoreNode tile_store = { NULL, NULL };
  if (load_tiles(&tile_store)) {
    dp("Failed on load_tiles.\n");
    goto cleanup;
  }

  Level* level;
  make_level_from_file("./level0.txt", &tile_store, &level);
  print_level(level);
  
  /** Main part */
  place_stay_level(level, screen_surface);
  SDL_UpdateWindowSurface(window);
  
  if (set_walk_object_to_direction(
    level->cells[1][1]->objects[1],
    DOWN,
    level->cells[1][2]
  ) == 0) {
    dp("Walk hero down.\n");
    
    Object* obj = level->cells[1][1]->objects[1];
    while (animate_walk_frame(obj)) {
      place_stay_level(level, screen_surface);
      place_walk_level(level, screen_surface);
      SDL_UpdateWindowSurface(window);
      SDL_Delay(1);
    }
    
    swap_object_between_cells(obj, level->cells[1][1], level->cells[1][2]);
  }
  
  place_stay_level(level, screen_surface);
  SDL_UpdateWindowSurface(window);
  
  SDL_Delay(10);
  
  // Result
  assert(level->cells[1][2]->objects[1]->type == OBJECT_HERO);

cleanup:
  destroy_sdl(&window, &screen_surface);
}

