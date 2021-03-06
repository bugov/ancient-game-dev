#include "game.h"

int load_tiles(Context* ctx) {
  int success = 0;
  Tile* tile = NULL;
  
// Q&D shortcut for tile adding
#define add_tile(...) do { success |= make_tile(__VA_ARGS__, ctx->renderer, &tile);\
  add_to_tile_store(ctx->tile_store, tile); } while (0)
  
  add_tile("attack-off",    "./tiles/attack-off.png");
  add_tile("attack-on",     "./tiles/attack-on.png");
  add_tile("grass",         "./tiles/grass.png");
  add_tile("wall",          "./tiles/wall.png");
  add_tile("human_attack",  "./tiles/human_attack.png");
  add_tile("human_walk",    "./tiles/human_walk.png");
  add_tile("human",         "./tiles/human.png");
  add_tile("hero_attack",   "./tiles/human_attack.png");
  add_tile("hero_walk",     "./tiles/human_walk.png");
  add_tile("hero",          "./tiles/human.png");
  add_tile("barrel",        "./tiles/barrel.png");
  add_tile("door",          "./tiles/door.png");
  add_tile("inventory",     "./tiles/inventory.png");
  add_tile("foilhat",       "./tiles/foilhat.png");
  add_tile("water",         "./tiles/water.png");
  add_tile("bridge",        "./tiles/bridge.png");
  add_tile("dark",          "./tiles/dark.png");
  
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
  
  dp("Click at rel px: %d;%d pos %d;%d\n",
    x_rel_px, y_rel_px, x_rel_pos, y_rel_pos);
  
  int x_pos = ctx->hero->x_pos + x_rel_pos;
  int y_pos = ctx->hero->y_pos + y_rel_pos;
  dp("Click at pos %d;%d\n", x_pos, y_pos);
  
  if (x_pos < 0 || y_pos < 0 || x_pos >= ctx->level->w || y_pos >= ctx->level->h) {
    dp("Click out of level!\n");
    return;
  }
  Cell* target_cell = ctx->level->cells[x_pos][y_pos];
  Object* target_obj = target_cell->objects[target_cell->depth - 1];
  
  if (ctx->mode == MODE_ATTACK) {
    // Reached target
    if (abs(x_rel_pos) < 2 && abs(y_rel_pos) < 2) {
      dp("Attack `%s` at pos %u;%u", target_obj->tile->name, target_obj->x_pos, target_obj->y_pos);
      set_attack_object(ctx, ctx->hero, target_obj);
      
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
  }
  
  // Handle inventory mouse clicks.
  else if (ctx->mode == MODE_INVENTORY) {
    handle_inventory_clicks(ctx, x_rel_px, y_rel_px);
  }
  
  else { // MODE_NORMAL
    // "Walking!" (c) some awesome George
    // No Dijkstra, just stupid walk
    if (target_obj->passable) {
      int dx = ctx->hero->x_pos - target_obj->x_pos;
      int dy = ctx->hero->y_pos - target_obj->y_pos;
      int blocked = 0;
      
      if (abs(dx) < abs(dy)) {
        if (dy < 0) blocked = set_walk_object_to_direction(ctx, ctx->hero, DOWN);
        else blocked = set_walk_object_to_direction(ctx, ctx->hero, UP);
        
        if (blocked) {
          if (dx < 0) set_walk_object_to_direction(ctx, ctx->hero, RIGHT);
          else set_walk_object_to_direction(ctx, ctx->hero, LEFT);
        }
      } else {
        if (dx < 0) blocked = set_walk_object_to_direction(ctx, ctx->hero, RIGHT);
        else blocked = set_walk_object_to_direction(ctx, ctx->hero, LEFT);
        
        if (blocked) {
          if (dy < 0) set_walk_object_to_direction(ctx, ctx->hero, DOWN);
          else set_walk_object_to_direction(ctx, ctx->hero, UP);
        }
      }
    }
    
    if (abs(x_rel_pos) > 1 || abs(y_rel_pos) > 1) return;
    
    if (target_obj->takeable) {
      take_object_into_inventory(ctx, target_obj, ctx->hero);
      return;
    }
    
    switch (target_obj->type) {
      case OBJECT_DOOR:
        target_obj->animation_frame ^= 1; // closed / opened
        target_obj->passable ^= 1;
        break;
      default:
        return;
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
  
  update_animation(ctx);
  render_level(ctx);
  SDL_Delay(SECOND / FPS);
  
  //render_iterface(ctx);
  //render_object_message(ctx, ctx->hero);
  
  if (ctx->mode == MODE_INVENTORY) {
    render_inventory(ctx);
  }
  
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
        case SDLK_i:
          dp("Toggle inventory mode\n");
          if (ctx->mode == MODE_NORMAL) ctx->mode = MODE_INVENTORY;
          else ctx->mode = MODE_NORMAL;
          break;
        case SDLK_ESCAPE:
          if (ctx->mode == MODE_NORMAL) {
            ctx->is_running = 0;
          } else {
            ctx->mode = MODE_NORMAL;
          }
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
    .window_width = 824,
    .window_height = 580,
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
  
  Object* hero = level->cells[5][4]->objects[1];
  dp("Get a hero: %u\n", hero->type);
  ctx.level = level;
  ctx.hero = hero;
  ctx.selected_slot = NULL;
  
  fix_objects_tile(&ctx);
  
  Tile* tmp_tile = NULL;
  find_in_tile_store(ctx.tile_store, "inventory", &tmp_tile);
  ctx.inventory_tile = tmp_tile;
  
  SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
  
  /** Main part */
  
  while (ctx.is_running) {
    render_loop(&ctx, window);
    handle_events(&ctx);
  }

cleanup:
  destroy_sdl(&window, &renderer);
}

