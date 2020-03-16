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


void render_walk(
  SDL_Window* window,
  SDL_Surface* screen_surface,
  Level* level
) {
  for (int x = 0; x < level->w; ++x) {
    for (int y = 0; y < level->h; ++y) {
      for (int z = 0; z < level->cells[x][y]->depth; ++z) {
        Object* obj = level->cells[x][y]->objects[z];
        if (obj->state == STATE_WALK) {
          dp("`%s` walks `%u`.\n", obj->tile->name, obj->direction);
          int dx = 0;
          int dy = 0;
          
          switch (obj->direction) {
            case UP: dy = -1; break;
            case DOWN: dy = 1; break;
            case LEFT: dx = -1; break;
            case RIGHT: dx = 1; break;
          }
          
          while (animate_walk_frame(obj)) {
            place_stay_level(level, screen_surface);
            place_walk_object(level, screen_surface, obj, x, y);
            SDL_UpdateWindowSurface(window);
            SDL_Delay(FPS);
          }
          
          swap_object_between_cells(obj, level->cells[x][y], level->cells[x + dx][y + dy]);
        }
      }
    }
  }
}


void render_loop(
  SDL_Window* window,
  SDL_Surface* screen_surface,
  Level* level
) {
  place_stay_level(level, screen_surface);
  SDL_UpdateWindowSurface(window);
  
  render_walk(window, screen_surface, level);
  
  place_stay_level(level, screen_surface);
  SDL_UpdateWindowSurface(window);
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
  Object* hero = level->cells[1][1]->objects[1];
  
  /** Main part */
  char is_running = 1;
  int mouse_x, mouse_y;
  SDL_Event event;
  
  while (is_running) {
    SDL_Delay(FPS);
    render_loop(window, screen_surface, level);
    
    if (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        is_running = 0;
      } else if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
          case SDLK_UP:
            set_walk_object_to_direction(hero, UP, level->cells[1][0]);
            break;
          case SDLK_DOWN:
            set_walk_object_to_direction(hero, DOWN, level->cells[1][2]);
            break;
          case SDLK_LEFT:
            set_walk_object_to_direction(hero, LEFT, level->cells[0][1]);
            break;
          case SDLK_RIGHT:
            set_walk_object_to_direction(hero, RIGHT, level->cells[2][1]);
            break;
          case SDLK_ESCAPE:
            is_running = 0;
            break;
        }
      } else if (
        event.type == SDL_MOUSEMOTION
        || event.type == SDL_MOUSEBUTTONDOWN
        || event.type == SDL_MOUSEBUTTONUP
      ) {
        SDL_GetMouseState(&mouse_x, &mouse_y);
      }
    }
  }

cleanup:
  destroy_sdl(&window, &screen_surface);
}

