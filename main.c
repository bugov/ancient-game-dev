#include "level.h"
#include "sdlike.h"


int main(void) {
  SDL_Window* window = NULL;
  SDL_Surface* screen_surface = NULL;
  
  if (init_sdl("Hello SDL", 640, 640, &window, &screen_surface)) {
  #ifdef DEBUG
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
  #endif
    goto cleanup;
  }
  
  Level* level;
  make_level_from_file("./level0.txt", &level);
  #ifdef DEBUG
		printf("Level was readed");
    print_level(level);
  #endif
  
  /** Main part */
  place_level(level, screen_surface);
  
  SDL_UpdateWindowSurface(window);
	
	SDL_Delay(1000);

cleanup:
  destroy_sdl(&window, &screen_surface);
}
