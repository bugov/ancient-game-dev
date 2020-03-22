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
  
  success |= make_tile("human_walk", "./tiles/human_walk.png", ctx->renderer, &tile);
  add_to_tile_store(ctx->tile_store, tile);
  
  success |= make_tile("human", "./tiles/human.png", ctx->renderer, &tile);
  add_to_tile_store(ctx->tile_store, tile);
  
  success |= make_tile("hero_walk", "./tiles/human_walk.png", ctx->renderer, &tile);
  add_to_tile_store(ctx->tile_store, tile);
  
  success |= make_tile("hero", "./tiles/human.png", ctx->renderer, &tile);
  add_to_tile_store(ctx->tile_store, tile);
  
  success |= make_tile("barrel", "./tiles/barrel.png", ctx->renderer, &tile);
  add_to_tile_store(ctx->tile_store, tile);
  
  success |= make_tile("door", "./tiles/door.png", ctx->renderer, &tile);
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

  if (SDL_RenderCopy(ctx->renderer, attack->image, NULL, &dst_rect)) {
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
          
          if (! update_walk_frame(ctx, obj)) {
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
          }
        }
      }
    }
  }
}


void handle_click(Context* ctx, int x_px, int y_px) {
  int x_rel_px = x_px - ctx->window_width / 2 - CELL_WIDTH / 2;
  int y_rel_px = y_px - ctx->window_height / 2 - CELL_HEIGHT / 2;
  int x_rel_pos = x_rel_px / CELL_WIDTH;
  int y_rel_pos = y_rel_px / CELL_HEIGHT;
  
  // fix pos
  if (x_px > ctx->window_width / 2 + CELL_WIDTH / 2) {
    x_rel_pos += 1;
  }
  if (y_px > ctx->window_height / 2 + CELL_HEIGHT / 2) {
    y_rel_pos += 1;
  }
  
  dp("Click at rel pos %d;%d\n", x_rel_pos, y_rel_pos);
  
  int x_pos = ctx->hero->x_pos + x_rel_pos;
  int y_pos = ctx->hero->y_pos + y_rel_pos;
  dp("Click at pos %d;%d\n", x_pos, y_pos);
  
  Cell* target_cell = ctx->level->cells[x_pos][y_pos];
  Object* target_obj = target_cell->objects[target_cell->depth - 1];
  
  // Click on hero
  if (x_rel_pos == 0 && y_rel_pos == 0) {
    return;
  }
  
  if (ctx->mode == MODE_ATTACK) {
    // Reached target
    if (abs(x_rel_pos) < 2 && abs(y_rel_pos) < 2) {
      dp("Attack `%s` at pos %u;%u", target_obj->tile->name, target_obj->x_pos, target_obj->y_pos);
      attack_object(ctx->hero, target_obj);
      
      if (target_obj->hp < 0) {
        remove_object_from_cell(target_obj, target_cell);
      }
      ctx->mode = MODE_NORMAL;
    }
  }
  else if (ctx->mode == MODE_TALK) {
    if (abs(x_rel_pos) < 2 && abs(y_rel_pos) < 2) {
      dp("Talk with `%s` at pos %u;%u", target_obj->tile->name, target_obj->x_pos, target_obj->y_pos);
      
      ctx->mode = MODE_NORMAL;
    }
  } else { // MODE_NORMAL
      // door
      switch (target_obj->type) {
        case OBJECT_DOOR:
          dp("Toggle door passable\n");
          target_obj->animation_frame ^= 1; // closed / opened
          target_obj->passable ^= 1;
      }
  }
}


int render_object_message(Context* ctx, Object* obj) {
  // XXX: Just test
  SDL_Rect dst_rect = {
    .x = obj->x_px - 50, // message offset
    .y = obj->y_px - 50,
    .w = obj->messages[0]->w,
    .h = obj->messages[0]->h
  };
  
  SDL_RenderCopy(ctx->renderer, obj->messages[0]->image, NULL, &dst_rect);
  return 0;
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
  //render_object_message(ctx, ctx->hero);
  
  SDL_RenderPresent(ctx->renderer);
}


void handle_events(Context* ctx) {
  SDL_Event event;
  int mouse_x, mouse_y;
  
  if (SDL_PollEvent(&event)) {
    if (ctx->is_busy) {
      SDL_FlushEvents(0, 0xffffffff); // all
      return;
    }
    
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
          dp("Toggle attack mode\n");
          if (ctx->mode == MODE_NORMAL) ctx->mode = MODE_ATTACK;
          else ctx->mode = MODE_NORMAL;
          break;
        case SDLK_e:
          dp("Toggle talk mode\n");
          if (ctx->mode == MODE_NORMAL) ctx->mode = MODE_TALK;
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
    handle_events(&ctx);
  }

cleanup:
  destroy_sdl(&window, &renderer);
}

