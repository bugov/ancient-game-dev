#include "sdlike.h"
#include "assert.h"

void test_init_destroy_sdl() {
  SDL_Window* window = NULL;
  SDL_Surface* screen_surface = NULL;
  
  //init_sdl("Hello SDL", 640, 640, &window, &screen_surface);
  //SDL_Delay(100);
  //assert(window != NULL);
  //assert(screen_surface != NULL);
  
  //destroy_sdl(&window, &screen_surface);
  
  //assert(window == NULL);
  //assert(screen_surface == NULL);
}


void test_tile_store() {
  Tile* tile = NULL;
  make_tile("grass", "./tiles/grass.png", &tile);
  assert(tile != NULL);
  assert(tile->image != NULL);
  assert(strcmp(tile->name, "grass") == 0);
  
  printf("Tile is fine.\n");
  
  TileStoreNode store = { NULL, NULL };
  assert(add_to_tile_store(&store, tile) == 0);
  Tile* another_tile = NULL;
  assert(find_in_tile_store(&store, "grass", &another_tile) == 0);
  assert(strcmp(tile->name, another_tile->name) == 0);
  assert(tile == another_tile);
  
  assert(make_tile("wall", "./tiles/wall.png", &tile) == 0);
  assert(add_to_tile_store(&store, tile) == 0);
  assert(find_in_tile_store(&store, "wall", &another_tile) == 0);
  assert(strcmp(another_tile->name, "wall") == 0);
}


int main(void) {
  test_init_destroy_sdl();
  test_tile_store();

  return 0;
}
