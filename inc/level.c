#include "level.h"
#include "sdlike.h"


char* obj_type_to_str(ObjType type) {
  switch (type) {
    case OBJECT_GRASS: return "grass";
    case OBJECT_WALL: return "wall";
    case OBJECT_WALL_TOP: return "wall_top";
    case OBJECT_HERO: return "hero";
    case OBJECT_BARREL: return "barrel";
    default:
      dp("Unknown type for convering to string.\n");
      return "error";
  }
}


ObjType str_to_obj_type(char* str) {
  if (strcmp("grass", str) == 0) return OBJECT_GRASS;
  if (strcmp("wall_top", str) == 0) return OBJECT_WALL_TOP;
  if (strcmp("wall", str) == 0) return OBJECT_WALL;
  if (strcmp("hero", str) == 0) return OBJECT_HERO;
  if (strcmp("barrel", str) == 0) return OBJECT_BARREL;
  return OBJECT_ERROR;
}


int make_object(
  Context* ctx,
  ObjType type,
  unsigned x_pos,
  unsigned y_pos,
  //
  Object** obj_ptr
) {
  dp("Make object.\n");
  *obj_ptr = (Object*)malloc(sizeof(Object));
  Object* obj = *obj_ptr;
  obj->type = type;
  obj->state = STATE_STAY;
  obj->x_pos = x_pos;
  obj->y_pos = y_pos;
  obj->x_px = x_pos * CELL_WIDTH;
  obj->y_px = y_pos * CELL_HEIGHT;
  
  Tile* tile;
  char* obj_type_name = obj_type_to_str(type);
  if (find_in_tile_store(ctx->tile_store, obj_type_name, &tile)) {
    return 1;
  }
  obj->tile = tile;
  
  // Attack / HP
  obj->base_attack = 0;
  switch (type) {
    case OBJECT_HERO:
      obj->hp = 5;
      obj->base_attack = 2;
      break;
    case OBJECT_BARREL: obj->hp = 1; break;
    default: obj->hp = 10000;
  }
  
  // Passability
  switch (type) {
    case OBJECT_HERO:
    case OBJECT_BARREL:
    case OBJECT_WALL_TOP:
    case OBJECT_WALL: obj->passable = 0; break; // no
    default: obj->passable = 1;  // yes
  }
  
  // Walkability
  switch (type) {
    case OBJECT_HERO: obj->walkable = 1; break;  // yes
    default: obj->walkable = 0; // no
  }
  
  if (obj->walkable) {
    char walk_tile_name[255];
    strcpy(walk_tile_name, obj_type_name);
    strcat(walk_tile_name, "_walk");
    dp("Stand up, pick up your mat, and walk, `%s`!\n", walk_tile_name);
    
    if (find_in_tile_store(ctx->tile_store, walk_tile_name, &tile)) {
      dp("Can't find tile for walking.\n");
      return 1;
    }
    obj->walk_tile = tile;
    obj->direction = DOWN;
    obj->walk_frame = 0;
  }
  
  return 0;
}


int set_walk_object_to_direction(
  Context* ctx,
  Object* obj,
  ObjDirection direction
) {
  if (! obj->walkable) {
    dp("Object isn't walkable.\n");
    return 1;
  }
  
  unsigned dx = 0;
  unsigned dy = 0;
  
  switch (direction) {
    case UP: dy = -1; break;
    case DOWN: dy = 1; break;
    case LEFT: dx = -1; break;
    case RIGHT: dx = 1; break;
  }
  
  Cell* cell = ctx->level->cells[obj->x_pos + dx][obj->y_pos + dy];
  if (! is_cell_passable(cell)) {
    dp("Cell %u;%u isn't passable.\n", obj->x_pos + dx, obj->y_pos + dy);
    return 2;
  }
  
  obj->state = STATE_WALK;
  obj->direction = direction;
  ctx->is_busy += 1;
  
  return 0;
}


int update_walk_frame(Object* obj) {
  if (obj->walk_frame >= WALK_FRAMES) {
    obj->state = STATE_STAY;
    obj->walk_frame = 0;
    return 0;
  }
  
  dp("Object walks...\n");
  obj->walk_frame += 1;
  
  unsigned dx = 0;
  unsigned dy = 0;
  
  switch (obj->direction) {
    case UP: dy = -1; break;
    case DOWN: dy = 1; break;
    case LEFT: dx = -1; break;
    case RIGHT: dx = 1; break;
  }
  
  obj->x_px = obj->x_pos * CELL_WIDTH + dx * (obj->walk_frame * WALK_STEP_PX);
  obj->y_px = obj->y_pos * CELL_HEIGHT + dy * (obj->walk_frame * WALK_STEP_PX);
  
  return 1;
}


int make_cell_from_char(
  Context* ctx,
  char cell_symbol,
  unsigned x_pos,
  unsigned y_pos,
  //
  Cell** cell_ptr
) {
  int success = 0;
  *cell_ptr = (Cell*)malloc(sizeof(Cell));
  Cell* cell = *cell_ptr;
  cell->depth = 0;
  cell->capacity = MIN_CELL_CAPACITY;
  cell->objects = malloc(sizeof(Object*) * MIN_CELL_CAPACITY);
  cell->x_pos = x_pos;
  cell->y_pos = y_pos;
  cell->changed = 1;

  Object* obj;
  success |= make_object(ctx, OBJECT_GRASS, x_pos, y_pos, &obj);
  success |= push_object_to_cell(obj, cell);
  
  if (cell_symbol == '#') {
    success |= make_object(ctx, OBJECT_WALL, x_pos, y_pos, &obj);
    success |= push_object_to_cell(obj, cell);
  }
  else if (cell_symbol == '^') {
    success |= make_object(ctx, OBJECT_WALL_TOP, x_pos, y_pos, &obj);
    success |= push_object_to_cell(obj, cell);
  }
  else if (cell_symbol == '@') {
    success |= make_object(ctx, OBJECT_HERO, x_pos, y_pos, &obj);
    success |= push_object_to_cell(obj, cell);
  }
  else if (cell_symbol == 'B') {
    success |= make_object(ctx, OBJECT_BARREL, x_pos, y_pos, &obj);
    success |= push_object_to_cell(obj, cell);
  }
  
  return success;
}


int push_object_to_cell (
  Object* obj,
  Cell* cell
) {
  if (cell->depth >= cell->capacity) {
    dp("Stack limit reached: Depth - %u; Capacity - %u\n", cell->depth, cell->capacity);

    if (cell->capacity >= MAX_CELL_CAPACITY) {
      dp("Depth overflow");
    }

    // expand stack if the limit reached
    Object** old_stack = cell->objects;
    cell->capacity *= 2;
    cell->objects = (Object**)malloc(sizeof(Object*) * cell->capacity);
    
    memcpy(cell->objects, old_stack, cell->depth * sizeof(*old_stack));
    free(old_stack);
  }

  cell->objects[cell->depth] = obj;
  cell->depth += 1;
  cell->changed = 1;
  
  // Fix object pos
  obj->x_pos = cell->x_pos;
  obj->y_pos = cell->y_pos;
  
  return 0;
}


int remove_object_from_cell (
  Object* obj,
  Cell* cell
) {
  // from the top
  if (obj == cell->objects[cell->depth - 1]) {
    cell->objects[cell->depth - 1] = NULL;
    cell->depth -= 1;
    return 0;
  }
  
  // from the middle
  Object** new_stack = malloc(sizeof(Object*) * cell->capacity);
  int z = 0;
  while (obj != cell->objects[z]) {
    new_stack[z] = cell->objects[z];
    ++z;
    if (z == cell->depth) {
      return 1;
    }
  }
  
  for (; z < cell->depth; ++z) {
    new_stack[z - 1] = cell->objects[z];
  }
  
  Object** old_stack = cell->objects;
  cell->objects = new_stack;
  cell->depth -= 1;
  free(old_stack);
  cell->changed = 1;
  
  return 0;
}


int is_cell_passable(Cell* cell) {
  int passable = 1;
  for (int z = 0; z < cell->depth; ++z) {
    dp("Object `%s` passable: %u\n", cell->objects[z]->tile->name, cell->objects[z]->passable * 1);
    passable &= cell->objects[z]->passable;
  }
  return passable;
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


void attack_object(Object* src, Object* dst) {
  dst->hp -= src->base_attack;
  dp("`%s` attacked by `%s`. Result HP: `%d`.\n", dst->tile->name, src->tile->name, dst->hp);
}


int render_level(Context* ctx) {
  int success = 0;
  Object* obj = NULL;

  for (int x = 0; x < ctx->level->w; ++x) {
    for (int y = 0; y < ctx->level->h; ++y) {
      for (int z = 0; z < ctx->level->cells[x][y]->depth; ++z) {
        obj = ctx->level->cells[x][y]->objects[z];
        
        if (obj->state == STATE_STAY) {
          success |= render_surface_pos(x, y, obj->tile->image, ctx->renderer);
        }
      }
    }
  }
  
  // Walk
  for (int x = 0; x < ctx->level->w; ++x) {
    for (int y = 0; y < ctx->level->h; ++y) {
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
          unsigned to_x_px = obj->x_pos * CELL_WIDTH + dx_pos * obj->walk_frame * WALK_STEP_PX;
          unsigned to_y_px = obj->y_pos * CELL_WIDTH + dy_pos * obj->walk_frame * WALK_STEP_PX;
          
          unsigned from_x_px = obj->walk_frame * CELL_WIDTH;
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
  
  return success;
}


int swap_object_between_cells(Object* obj, Cell* src, Cell* dst) {
  int success = remove_object_from_cell(obj, src);
  return success | push_object_to_cell(obj, dst);
}

