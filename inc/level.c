#include "game.h"

#define MESSAGE_FONT_SIZE 24


int make_message(
  Context* ctx,
  char* text,
  //
  Message** message_ptr
) {
  TTF_Font* font = TTF_OpenFont("./assets/Anonymous.ttf", MESSAGE_FONT_SIZE);
  if (font == NULL) {
    dp("An error on font load: %s\n", SDL_GetError());
    return 1;
  }
  
  SDL_Color color = {255, 255, 255};
  SDL_Surface* message_surf = TTF_RenderText_Solid(font, text, color);
  if (message_surf == NULL) {
    dp("An error on creating font surface: %s\n", SDL_GetError());
    return 1;
  }
  
  SDL_Texture* message_texture = SDL_CreateTextureFromSurface(ctx->renderer, message_surf);
  if (message_texture == NULL) {
    dp("An error on creating font texture: %s\n", SDL_GetError());
    return 1;
  }
  
  SDL_FreeSurface(message_surf);
  
  *message_ptr = (Message*)malloc(sizeof(Message));
  Message* message = *message_ptr;
  message->image = message_texture;
  message->text = text;
  message->h = MESSAGE_FONT_SIZE;
  message->w = strlen(message->text) * MESSAGE_FONT_SIZE;
  
  return 0;
}


int make_level_from_file(
  Context* ctx,
  char* path,
  //
  Level** level_ptr
) {
  *level_ptr = (Level*)malloc(sizeof(Level));
  Level* level = *level_ptr;
  FILE* file = fopen(path, "r");
  unsigned w;
  unsigned h;
  
  if (fscanf(file, "%u %u\n", &w, &h) != 2) {
    dp("Can't read level size.\n");
    goto error;
  }
  
  level->w = w;
  level->h = h;
  
  dp("Init level with size: %u;%u.\n", w, h);
  
  Cell* cell = NULL;
  
  // TODO: check malloc errors
  // malloc all cells
  level->cells = (Cell***)malloc(sizeof(Cell**) * level->w);
  for (int x = 0; x < level->w; ++x) {
    level->cells[x] = (Cell**)malloc(sizeof(Cell*) * level->h);
  }
  
  // Get data
  for (int y = 0; y < level->h; ++y) {
    for (int x = 0; x < level->w; ++x) {
      char cell_symbol = fgetc(file);
      if (cell_symbol == EOF) {
        printf("Error on read pos %u;%u.\n", x, y);
        goto error;
      }
      dp("Read cell %u;%u symbol `%c`\n", x, y, cell_symbol);
      
      make_cell_from_char(ctx, cell_symbol, x, y, &cell);
      level->cells[x][y] = cell;
    }
    fgetc(file);  // slurp NL
  }

  fclose(file);
  return 0;
  
error:
  fclose(file);
  return 1;
}


/**
  Find buildings / locations (closed spaces)
*/
void fix_objects_tile(Context* ctx) {
  for (int x = 0; x < ctx->level->w; ++x) {
    for (int y = 0; y < ctx->level->h; ++y) {
      Object* obj = ctx->level->cells[x][y]->objects[ctx->level->cells[x][y]->depth - 1];
      // Multitile for walls
      if (obj->type == OBJECT_WALL && y < ctx->level->h - 1) {
        Cell* bottom_cell = ctx->level->cells[x][y + 1];
        if (bottom_cell->objects[bottom_cell->depth - 1]->type == OBJECT_WALL) {
          dp("Fix wall tile on %u;%u\n", x, y);
          obj->direction = 1; // wall from top
        }
      }
      
      // Multitile for grass / floor.
      if (obj->type == OBJECT_GRASS || obj->type == OBJECT_WATER) {
        obj->animation_frame = abs(x*y + y*y + rand()) % 4;
      }
      
      // Find rooms - rects left2right top2bottom
      if (
        obj->type == OBJECT_WALL
        && y < ctx->level->h - 1
        && x < ctx->level->w - 1
      ) {
        Cell*** cells = ctx->level->cells;
        // find corner
        Object* r_obj = cells[x+1][y]->objects[cells[x+1][y]->depth - 1];
        Object* b_obj = cells[x][y+1]->objects[cells[x][y+1]->depth - 1];
        Object* rb_obj = cells[x+1][y+1]->objects[cells[x+1][y+1]->depth - 1];
        
        if (
          r_obj->type == OBJECT_WALL
          && b_obj->type == OBJECT_WALL
          && rb_obj->type != OBJECT_WALL
        ) {
          dp("Find left-top house corner at pos: %d;%d\n", x, y);
          int xx = x;
          int yy = y;
          
          while (
            xx < ctx->level->w
            && (
              cells[xx][y]->objects[cells[xx][y]->depth - 1]->type == OBJECT_WALL
              || cells[xx][y]->objects[cells[xx][y]->depth - 1]->type == OBJECT_DOOR
            )
          ) {
            xx++;
          }
          while (
            yy < ctx->level->h
            && (
              cells[x][yy]->objects[cells[x][yy]->depth - 1]->type == OBJECT_WALL
              || cells[x][yy]->objects[cells[x][yy]->depth - 1]->type == OBJECT_DOOR
            )
          ) {
            yy++;
          }
          
          dp("Find right-bottom house corner at pos: %d;%d\n", xx, yy);
          
          for (int tx = x; tx < xx; ++tx) {
            for (int ty = y; ty < yy; ++ty) {
              for (int z = 0; z < cells[tx][ty]->depth; ++z) {
                if (cells[tx][ty]->objects[z]->type == OBJECT_GRASS) {
                  dp("Fix grass tile on pos %d;%d\n", tx, ty);
                  cells[tx][ty]->objects[z]->direction = 1;
                }
              }
            }
          }
        }
      }
    }
  }
}


int render_level(Context* ctx) {
  int success = 0;
  Object* obj = NULL;
  Tile* dark_tile = NULL;
  find_in_tile_store(ctx->tile_store, "dark", &dark_tile);

  for (
    int x = max(0, (int)(ctx->hero->x_pos) - RERENDER_RANGE);
        x < min(ctx->hero->x_pos + RERENDER_RANGE, ctx->level->w);
        ++x
  ) {
    for (
      int y = max(0, (int)(ctx->hero->y_pos) - RERENDER_RANGE);
          y < min(ctx->hero->y_pos + RERENDER_RANGE, ctx->level->h);
          ++y
    ) {
      for (int z = 0; z < ctx->level->cells[x][y]->depth; ++z) {
        obj = ctx->level->cells[x][y]->objects[z];
        
        if (obj->state == STATE_STAY) {
          if (! obj->walkable) {
            success |= render_surface_part_pos(
              obj->animation_frame,
              obj->direction,
              x,
              y,
              obj->tile->image,
              ctx->renderer
            );
          }
          else {
            success |= render_surface_part_pos(
              obj->animation_frame,
              obj->direction,
              obj->x_pos,
              obj->y_pos,
              obj->walk_tile->image,
              ctx->renderer
            );
          }
        }
      }
    }
  }
  
  // Walk
  for (
    int x = max(0, (int)(ctx->hero->x_pos) - RERENDER_RANGE);
        x < min(ctx->hero->x_pos + RERENDER_RANGE, ctx->level->w);
        ++x
  ) {
    for (
      int y = max(0, (int)(ctx->hero->y_pos) - RERENDER_RANGE);
          y < min(ctx->hero->y_pos + RERENDER_RANGE, ctx->level->h);
          ++y
    ) {
      for (int z = 0; z < ctx->level->cells[x][y]->depth; ++z) {
        obj = ctx->level->cells[x][y]->objects[z];
        
        if (obj->state == STATE_WALK) {
          int dx_pos = 0;
          int dy_pos = 0;
          switch (obj->direction) {
            case UP: dy_pos = -1; break;
            case DOWN: dy_pos = 1; break;
            case LEFT: dx_pos = -1; break;
            case RIGHT: dx_pos = 1; break;
          }
          unsigned to_x_px = obj->x_pos * CELL_WIDTH + dx_pos * obj->animation_frame * WALK_STEP_PX;
          unsigned to_y_px = obj->y_pos * CELL_WIDTH + dy_pos * obj->animation_frame * WALK_STEP_PX;
          
          unsigned from_x_px = obj->animation_frame * CELL_WIDTH;
          unsigned from_y_px = obj->direction * CELL_HEIGHT;
          
          success |= render_surface_part_px(
            from_x_px,
            from_y_px,
            to_x_px,
            to_y_px,
            obj->walk_tile->image,
            ctx->renderer
          );
        }
      }
    }
  }
  
  // Attack
  for (
    int x = max(0, (int)(ctx->hero->x_pos) - RERENDER_RANGE);
        x < min(ctx->hero->x_pos + RERENDER_RANGE, ctx->level->w);
        ++x
  ) {
    for (
      int y = max(0, (int)(ctx->hero->y_pos) - RERENDER_RANGE);
          y < min(ctx->hero->y_pos + RERENDER_RANGE, ctx->level->h);
          ++y
    ) {
      for (int z = 0; z < ctx->level->cells[x][y]->depth; ++z) {
        obj = ctx->level->cells[x][y]->objects[z];
        
        if (obj->state == STATE_ATTACK) {
          success |= render_surface_part_px(
            obj->animation_frame * CELL_WIDTH,
            obj->direction * CELL_HEIGHT,
            obj->x_px,
            obj->y_px,
            obj->attack_tile->image,
            ctx->renderer
          );
        }
      }
    }
  }
  
  return success;
}


void update_animation(Context* ctx) {
  Level* level = ctx->level;
  
  for (
    int x = max(0, (int)(ctx->hero->x_pos) - RERENDER_RANGE);
        x < min(ctx->hero->x_pos + RERENDER_RANGE, ctx->level->w);
        ++x
  ) {
    for (
      int y = max(0, (int)(ctx->hero->y_pos) - RERENDER_RANGE);
          y < min(ctx->hero->y_pos + RERENDER_RANGE, ctx->level->h);
          ++y
    ) {
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
        else if (obj->state == STATE_ATTACK) {
          update_attack_frame(ctx, obj);
        }
      }
    }
  }
}


int swap_object_between_cells(Object* obj, Cell* src, Cell* dst) {
  int success = remove_object_from_cell(obj, src);
  return success | push_object_to_cell(obj, dst);
}

