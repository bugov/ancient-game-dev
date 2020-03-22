#include "level.h"
#include "sdlike.h"


int load_tiles(Context* ctx) {
  int success = 0;
  Tile* tile = NULL;
  
  success |= make_tile("attack-off", "./tiles/attack-off.png", ctx->renderer, &tile);
  add_to_tile_store(ctx->tile_store, tile);
  
  success |= make_tile("attack-on", "./tiles/attack-on.png", ctx->renderer, &tile);
  add_to_tile_store(ctx->tile_store, tile);
  
  success |= make_tile("grass", "./tiles/grass.png", ctx->renderer, &tile);
  add_to_tile_store(ctx->tile_store, tile);
  
  success |= make_tile("wall", "./tiles/wall.png", ctx->renderer, &tile);
  add_to_tile_store(ctx->tile_store, tile);
  
  success |= make_tile("wall_top", "./tiles/wall_top.png", ctx->renderer, &tile);
  add_to_tile_store(ctx->tile_store, tile);
  
  success |= make_tile("hero_walk", "./tiles/hero_walk.png", ctx->renderer, &tile);
  add_to_tile_store(ctx->tile_store, tile);
  
  success |= make_tile("hero", "./tiles/hero.png", ctx->renderer, &tile);
  add_to_tile_store(ctx->tile_store, tile);
  
  success |= make_tile("barrel", "./tiles/barrel.png", ctx->renderer, &tile);
  add_to_tile_store(ctx->tile_store, tile);
  
  return success;
}


void render_iterface(Context* ctx) {
  Tile* attack = NULL;
  if (ctx->mode == MODE_ATTACK) {
    find_in_tile_store(ctx->tile_store, "attack-on", &attack);
  }
  else {
    find_in_tile_store(ctx->tile_store, "attack-off", &attack);
  }
  
  SDL_Rect dst_rect = {
    .x = ctx->window_width - ICON_SIZE - 10, // 10px margin
    .y = 50, // Just a random y-offset
    .w = ICON_SIZE,
    .h = ICON_SIZE,
  };

  if (SDL_RenderCopyEx(ctx->renderer, attack->image, NULL, &dst_rect, 0, 0, SDL_FLIP_NONE)) {
    dp("Can't render a texture. SDL_Error: %s\n", SDL_GetError());
  }
}


void update_walkers(
  Context* ctx,
  SDL_Window* window
) {
  Level* level = ctx->level;
  
  for (int x = 0; x < level->w; ++x) {
    for (int y = 0; y < level->h; ++y) {
      for (int z = 0; z < level->cells[x][y]->depth; ++z) {
        Object* obj = level->cells[x][y]->objects[z];
        
        if (obj->state == STATE_WALK) {
          dp("`%s` walks `%u`.\n", obj->tile->name, obj->direction);
          
          if (! update_walk_frame(obj)) {
            int dx = 0;
            int dy = 0;
            
            switch (obj->direction) {
              case UP: dy = -1; break;
              case DOWN: dy = 1; break;
              case LEFT: dx = -1; break;
              case RIGHT: dx = 1; break;
            }
            
            swap_object_between_cells(
              obj,
              level->cells[x][y],
              level->cells[x + dx][y + dy]
            );
            
            ctx->is_busy -= 1;
          }
        }
      }
    }
  }
}


void handle_click(Context* ctx, unsigned x_px, unsigned y_px) {
  int x_pos = x_px / CELL_WIDTH;
  int y_pos = y_px / CELL_HEIGHT;
  dp("Click at pos %u;%u", x_pos, y_pos);
  
  if (ctx->mode == MODE_ATTACK) {
    // Reached target
    // TODO: Check int overflow (is it necessary for pos?)
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
  Context* ctx,
  SDL_Window* window
) {
  
  set_viewport(
    (ctx->window_width - CELL_WIDTH) / 2 - ctx->hero->x_px,
    (ctx->window_height - CELL_HEIGHT) / 2 - ctx->hero->y_px,
    ctx->level->w * CELL_WIDTH,
    ctx->level->h * CELL_HEIGHT,
    ctx->renderer
  );
  
  SDL_RenderClear(ctx->renderer);
  SDL_SetRenderDrawColor(ctx->renderer, 44, 44, 44, 44);
  
  update_walkers(ctx, window);
  render_level(ctx);
  render_iterface(ctx);
  
  SDL_RenderPresent(ctx->renderer);
}


void handle_events(Context* ctx) {
  SDL_Event event;
  int mouse_x, mouse_y;
  
  if (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      ctx->is_running = 0;
    } else if (event.type == SDL_KEYDOWN) {
      switch (event.key.keysym.sym) {
        case SDLK_w:
        case SDLK_UP:
          set_walk_object_to_direction(ctx, ctx->hero, UP);
          break;
        case SDLK_s:
        case SDLK_DOWN:
          set_walk_object_to_direction(ctx, ctx->hero, DOWN);
          break;
        case SDLK_a:
        case SDLK_LEFT:
          set_walk_object_to_direction(ctx, ctx->hero, LEFT);
          break;
        case SDLK_d:
        case SDLK_RIGHT:
          set_walk_object_to_direction(ctx, ctx->hero, RIGHT);
          break;
        case SDLK_q:
          dp("Enter to attack mode\n");
          if (ctx->mode == MODE_NORMAL) ctx->mode = MODE_ATTACK;
          else ctx->mode = MODE_NORMAL;
          break;
        case SDLK_ESCAPE:
          ctx->is_running = 0;
          break;
      }
    } else if (event.type == SDL_MOUSEBUTTONDOWN)
    //  event.type == SDL_MOUSEMOTION
    //  event.type == SDL_MOUSEBUTTONUP
    {
      SDL_GetMouseState(&mouse_x, &mouse_y);
      dp("Got a click: %u;%u\n", mouse_x, mouse_y);
      handle_click(ctx, mouse_x, mouse_y);
    }
  }
}


int main(void) {
  Context ctx = {
    .mode = MODE_NORMAL,
    .window_width = 640,
    .window_height = 480,
    .is_running = 1,
    .is_busy = 0
  };
  
  SDL_Window* window = NULL;
  SDL_Renderer* renderer = NULL;
  
  if (init_sdl(
    "Ancient GameDev",
    ctx.window_width,
    ctx.window_height,
    &window,
    &renderer
  )) {
    dp("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    goto cleanup;
  }
  ctx.renderer = renderer;
  
  // SDL_RenderSetScale(renderer, 0.5, 0.5);
  
  TileStoreNode tile_store = { NULL, NULL };
  ctx.tile_store = &tile_store;
  if (load_tiles(&ctx)) {
    dp("Failed on load_tiles.\n");
    goto cleanup;
  }

  Level* level;
  make_level_from_file(&ctx, "./level0.txt", &level);
  Object* hero = level->cells[1][1]->objects[1];
  ctx.level = level;
  ctx.hero = hero;
  
  SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
  /** Main part */
  
  while (ctx.is_running) {
    SDL_Delay(SECOND / FPS);
    render_loop(&ctx, window);
    
    if (! ctx.is_busy) {
      handle_events(&ctx);
    }
  }

cleanup:
  destroy_sdl(&window, &renderer);
}

