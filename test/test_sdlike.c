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
  add_to_tile_store(&store, tile);
  Tile* another_tile = NULL;
  find_in_tile_store(&store, "grass", &another_tile);
  assert(strcmp(tile->name, another_tile->name) == 0);
  assert(tile->name == another_tile->name);
}


int main(void) {
  test_init_destroy_sdl();
  test_tile_store();

  return 0;
}
