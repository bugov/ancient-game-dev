#include "level.h"
#include "sdlike.h"


typedef enum GameMode {
  MODE_NORMAL,
  MODE_ATTACK
} GameMode;


typedef struct Context {
  GameMode mode;
  TileStoreNode* tile_store;
  Level* level;
  Object* hero;
} Context;


int load_tiles(TileStoreNode* tile_store) {
  int success = 0;
  Tile* tile = NULL;
  
  success |= make_tile("attack-off", "./tiles/attack-off.png", &tile);
  add_to_tile_store(tile_store, tile);
  
  success |= make_tile("attack-on", "./tiles/attack-on.png", &tile);
  add_to_tile_store(tile_store, tile);
  
  success |= make_tile("grass", "./tiles/grass.png", &tile);
  add_to_tile_store(tile_store, tile);
  
  success |= make_tile("wall", "./tiles/wall.png", &tile);
  add_to_tile_store(tile_store, tile);
  
  success |= make_tile("hero_walk", "./tiles/hero_walk.png", &tile);
  add_to_tile_store(tile_store, tile);
  
  success |= make_tile("hero", "./tiles/hero.png", &tile);
  add_to_tile_store(tile_store, tile);
  
  success |= make_tile("barrel", "./tiles/barrel.png", &tile);
  add_to_tile_store(tile_store, tile);
  
  return success;
}


void render_iterface(
  SDL_Window* window,
  SDL_Surface* screen_surface,
  Context* ctx,
  unsigned window_width
) {
  Tile* attack = NULL;
  if (ctx->mode == MODE_ATTACK) {
    find_in_tile_store(ctx->tile_store, "attack-on", &attack);
  }
  else {
    find_in_tile_store(ctx->tile_store, "attack-off", &attack);
  }
  
  SDL_Rect dst_rect = {
    .x = window_width - ICON_SIZE - 10,  // 10px margin
    .y = 50, // Just a random y-offset
    .w = ICON_SIZE,
    .h = ICON_SIZE,
  };

  if (SDL_BlitSurface(attack->image, NULL, screen_surface, &dst_rect)) {
    dp("Can't place a surface. SDL_Error: %s\n", SDL_GetError());
  }
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
            place_walk_object(level, screen_surface, obj);
            SDL_UpdateWindowSurface(window);
            SDL_Delay(FPS);
          }
          
          swap_object_between_cells(obj, level->cells[x][y], level->cells[x + dx][y + dy]);
        }
      }
    }
  }
}


void handle_click(unsigned x_px, unsigned y_px, Context* ctx) {
  int x_pos = x_px / CELL_WIDTH;
  int y_pos = y_px / CELL_HEIGHT;
  dp("Click at pos %u;%u", x_pos, y_pos);
  
  if (ctx->mode == MODE_ATTACK) {
    // Reached target
    // TODO: Check int overflow
    if (
      abs((int)ctx->hero->x_pos - x_pos) < 2
      && abs((int)ctx->hero->y_pos - y_pos) < 2
    ) {
      Cell* target_cell = ctx->level->cells[x_pos][y_pos];
      Object* target_obj = target_cell->objects[target_cell->depth - 1];
      dp("Attack `%s` at pos %u;%u", target_obj->tile->name, target_obj->x_pos, target_obj->y_pos);
      attack_object(ctx->hero, target_obj);
      
      if (target_obj->hp < 0) {
        remove_object_from_cell(target_obj, target_cell);
      }
      ctx->mode = MODE_NORMAL;
    }
  }
}


void render_loop(
  SDL_Window* window,
  SDL_Surface* screen_surface,
  Context* ctx,
  unsigned window_width,
  Level* level
) {
  render_walk(window, screen_surface, level);
  
  place_stay_level(level, screen_surface);
  render_iterface(window, screen_surface, ctx, window_width);
  SDL_UpdateWindowSurface(window);
}


int main(void) {
  SDL_Window* window = NULL;
  SDL_Surface* screen_surface = NULL;
  
  SDL_Cursor* normal_cursor = SDL_GetCursor();
  SDL_Cursor* attack_cursor = NULL;
  make_cursor(ATTACK_CURSOR, &attack_cursor);

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
  
  Context ctx = {
    .mode = MODE_NORMAL,
    .tile_store = &tile_store,
    .level = level,
    .hero = hero
  };
  
  /** Main part */
  char is_running = 1;
  int mouse_x, mouse_y;
  SDL_Event event;
  
  while (is_running) {
    SDL_Delay(FPS);
    render_loop(window, screen_surface, &ctx, 640, level);
    
    if (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        is_running = 0;
      } else if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
          case SDLK_w:
          case SDLK_UP:
            set_walk_object_to_direction(hero, UP, level);
            break;
          case SDLK_s:
          case SDLK_DOWN:
            set_walk_object_to_direction(hero, DOWN, level);
            break;
          case SDLK_a:
          case SDLK_LEFT:
            set_walk_object_to_direction(hero, LEFT, level);
            break;
          case SDLK_d:
          case SDLK_RIGHT:
            set_walk_object_to_direction(hero, RIGHT, level);
            break;
          case SDLK_q:
            dp("Enter to attack mode\n");
            if (ctx.mode == MODE_NORMAL) ctx.mode = MODE_ATTACK;
            else ctx.mode = MODE_NORMAL;
            //SDL_SetCursor(attack_cursor);
            //SDL_ShowCursor(0);
            break;
          case SDLK_ESCAPE:
            is_running = 0;
            break;
        }
      } else if (event.type == SDL_MOUSEBUTTONDOWN)
      //  event.type == SDL_MOUSEMOTION
      //  event.type == SDL_MOUSEBUTTONUP
      {
        SDL_GetMouseState(&mouse_x, &mouse_y);
        dp("Got a click: %u;%u\n", mouse_x, mouse_y);
        handle_click(mouse_x, mouse_y, &ctx);
      }
    }
  }

cleanup:
  destroy_sdl(&window, &screen_surface);
}

