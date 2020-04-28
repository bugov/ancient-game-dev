#include "game.h"


char* obj_type_to_str(ObjType type) {
  switch (type) {
    case OBJECT_GRASS: return "grass";
    case OBJECT_DOOR: return "door";
    case OBJECT_WALL: return "wall";
    case OBJECT_HERO: return "hero";
    case OBJECT_HUMAN: return "human";
    case OBJECT_BARREL: return "barrel";
    case OBJECT_FOILHAT: return "foilhat";
    case OBJECT_WATER: return "water";
    case OBJECT_BRIDGE: return "bridge";
    default:
      dp("Unknown type for convering to string.\n");
      return "error";
  }
}


ObjType str_to_obj_type(char* str) {
  if (strcmp("grass", str) == 0) return OBJECT_GRASS;
  if (strcmp("door", str) == 0) return OBJECT_DOOR;
  if (strcmp("wall", str) == 0) return OBJECT_WALL;
  if (strcmp("hero", str) == 0) return OBJECT_HERO;
  if (strcmp("human", str) == 0) return OBJECT_HUMAN;
  if (strcmp("barrel", str) == 0) return OBJECT_BARREL;
  if (strcmp("foilhat", str) == 0) return OBJECT_FOILHAT;
  if (strcmp("water", str) == 0) return OBJECT_WATER;
  if (strcmp("bridge", str) == 0) return OBJECT_BRIDGE;
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
  obj->animation_frame = 0;
  obj->direction = 0;
  obj->slots = NULL;
  obj->messages = NULL;
  obj->equip = NULL;
  obj->visibility = 1;
  obj->base_visibility_range = 0;
  
  Tile* tile;
  char* obj_type_name = obj_type_to_str(type);
  if (find_in_tile_store(ctx->tile_store, obj_type_name, &tile)) {
    return 1;
  }
  obj->tile = tile;
  
  // Slots 
  if (obj->type == OBJECT_HERO) {
    obj->slots = (Slot***)malloc(sizeof(Slot**) * INV_SLOTS_WIDTH);
    
    for (int x = 0; x < INV_SLOTS_WIDTH; ++x) {
      obj->slots[x] = (Slot**)malloc(sizeof(Slot*) * INV_SLOTS_HEIGHT);
      
      for (int y = 0; y < INV_SLOTS_HEIGHT; ++y) {
        obj->slots[x][y] = (Slot*)malloc(sizeof(Slot));
        obj->slots[x][y]->obj_type = OBJECT_ERROR;
        obj->slots[x][y]->animation_frame = 0;
        obj->slots[x][y]->x_pos = x;
        obj->slots[x][y]->y_pos = y;
        obj->slots[x][y]->is_equip = 0;
      }
    }
    
    // XXX: FTGJ add foil hat
    Tile* tile = NULL;
    find_in_tile_store(ctx->tile_store, "foilhat", &tile);
    obj->slots[1][0]->tile = tile;
    obj->slots[1][0]->obj_type = OBJECT_FOILHAT;
    
    // Equipment
    obj->equip = (Slot***)malloc(sizeof(Slot**) * EQUIP_SLOTS_WIDTH);
    
    for (int x = 0; x < EQUIP_SLOTS_WIDTH; ++x) {
      obj->equip[x] = (Slot**)malloc(sizeof(Slot*) * EQUIP_SLOTS_HEIGHT);
      
      for (int y = 0; y < EQUIP_SLOTS_HEIGHT; ++y) {
        obj->equip[x][y] = (Slot*)malloc(sizeof(Slot));
        obj->equip[x][y]->obj_type = OBJECT_ERROR;
        obj->equip[x][y]->animation_frame = 0;
        obj->equip[x][y]->x_pos = x;
        obj->equip[x][y]->y_pos = y;
        obj->equip[x][y]->is_equip = 1;
      }
    }
    
    // XXX: FTGJ add foil hat
    find_in_tile_store(ctx->tile_store, "foilhat", &tile);
    obj->equip[1][0]->tile = tile;
    obj->equip[1][0]->obj_type = OBJECT_FOILHAT;
  }
  
  // Takeable
  switch (type) {
    case OBJECT_FOILHAT:
      obj->takeable = 1;
      break;
    default: obj->takeable = 0;
  }
  
  // Talks
  Message* msg_hello = NULL;
  make_message(ctx, "hello!", &msg_hello);
  
  switch (type) {
    case OBJECT_HUMAN:
    case OBJECT_HERO:
      obj->talkable = 1;
      break;
    default: obj->talkable = 0;
  }
  
  if (obj->talkable) { // XXX: free
    obj->messages = (Message**)malloc(sizeof(Message*) * 1); // message array
    obj->messages[0] = msg_hello;
  }
  
  // Attack / HP
  obj->base_attack = 0;
  switch (type) {
    case OBJECT_HUMAN:
    case OBJECT_HERO:
      obj->hp = 5;
      obj->base_attack = 2;
      break;
    case OBJECT_FOILHAT:
    case OBJECT_BARREL:
      obj->hp = 1;
      break;
    default: obj->hp = 10000;
  }
  
  // Visibility
  switch (type) {
    case OBJECT_HUMAN:
    case OBJECT_HERO:
      obj->base_visibility_range = 5;
      break;
    default: obj->base_visibility_range = 0;
  }
  switch (type) {
    case OBJECT_WALL:
    case OBJECT_DOOR:
      obj->visibility = 0;
      break;
    default: obj->visibility = 1;
  }
  
  // Passability
  switch (type) {
    case OBJECT_DOOR:
    case OBJECT_HERO:
    case OBJECT_HUMAN:
    case OBJECT_BARREL:
    case OBJECT_WATER:
    case OBJECT_WALL: obj->passable = 0; break; // no
    default: obj->passable = 1;  // yes
  }
  
  // Walkability
  switch (type) {
    case OBJECT_HUMAN:
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
    obj->animation_frame = 0;
  }
  
  if (obj->base_attack) {
    char attack_tile_name[255];
    strcpy(attack_tile_name, obj_type_name);
    strcat(attack_tile_name, "_attack");
    
    if (find_in_tile_store(ctx->tile_store, attack_tile_name, &tile)) {
      dp("Can't find tile for attack: %s.\n", attack_tile_name);
      return 1;
    }
    obj->attack_tile = tile;
  }
  
  return 0;
}


void free_object(Object* obj) {
  if (obj->slots != NULL) {
    for (int x = 0; x < INV_SLOTS_WIDTH; ++x) {
      for (int y = 0; y < INV_SLOTS_HEIGHT; ++y) {
        free(obj->slots[x][y]);
      }
      free(obj->slots[x]);
    }
    free(obj->slots);
  }
  obj->slots = NULL;
  
  if (obj->equip != NULL) {
    for (int x = 0; x < EQUIP_SLOTS_WIDTH; ++x) {
      for (int y = 0; y < EQUIP_SLOTS_HEIGHT; ++y) {
        free(obj->equip[x][y]);
      }
      free(obj->equip[x]);
    }
    free(obj->equip);
  }
  obj->equip = NULL;
  
  free(obj);
}


int set_attack_object(
  Context* ctx,
  Object* src,
  Object* dst
) {
  if (ctx->mode != MODE_ATTACK) return 1;
  if (src->base_attack < 1) return 2;
  // Melee attack
  // XXX: int overflow
  if (abs((int)src->x_pos - (int)dst->x_pos) + abs((int)src->y_pos - (int)dst->y_pos) > 1) return 3;
  
  ObjDirection direction = DOWN;
  if (src->x_pos > dst->x_pos) direction = LEFT;
  if (src->x_pos < dst->x_pos) direction = RIGHT;
  if (src->y_pos > dst->y_pos) direction = UP;
  if (src->y_pos < dst->y_pos) direction = DOWN;

  src->animation_frame = 0;
  src->state = STATE_ATTACK;
  src->direction = direction;
  ctx->is_busy += 1;
  
  dst->hp -= src->base_attack;
  dp("`%s` attacked by `%s`. Result HP: `%d`.\n", dst->tile->name, src->tile->name, dst->hp);
  
  return 0;
}


int update_attack_frame(Context* ctx, Object* obj) {
  if (obj->animation_frame >= ATTACK_FRAMES) {
    obj->state = STATE_STAY;
    obj->animation_frame = 0;
    ctx->is_busy -= 1;
    return 0;
  }
  
  dp("Object attacks...\n");
  obj->animation_frame += 1;
  
  return 1;
}


int set_walk_object_to_direction(
  Context* ctx,
  Object* obj,
  ObjDirection direction
) {
  if (ctx->mode != MODE_NORMAL) {
    return 1;
  }

  if (! obj->walkable) {
    dp("Object isn't walkable.\n");
    return 1;
  }
  
  int dx = 0;
  int dy = 0;
  
  switch (direction) {
    case UP: dy = -1; break;
    case DOWN: dy = 1; break;
    case LEFT: dx = -1; break;
    case RIGHT: dx = 1; break;
  }
  
  // Level borders
  if (
    (int)obj->x_pos + dx < 0
    || (int)obj->x_pos + dx > ctx->level->w
    || (int)obj->y_pos + dy < 0
    || (int)obj->y_pos + dy >= ctx->level->h
  ) {
    dp("Level border is not passable.\n");
    return 3;
  }
  
  Cell* cell = ctx->level->cells[obj->x_pos + dx][obj->y_pos + dy];
  if (! is_cell_passable(cell)) {
    dp("Cell %u;%u isn't passable.\n", obj->x_pos + dx, obj->y_pos + dy);
    return 2;
  }
  
  obj->animation_frame = 0;
  obj->state = STATE_WALK;
  obj->direction = direction;
  ctx->is_busy += 1;
  
  return 0;
}


int update_walk_frame(Context* ctx, Object* obj) {
  if (obj->animation_frame >= WALK_FRAMES) {
    obj->state = STATE_STAY;
    //obj->animation_frame = 0;
    ctx->is_busy -= 1;
    return 0;
  }
  
  dp("Object walks...\n");
  obj->animation_frame += 1;
  
  unsigned dx = 0;
  unsigned dy = 0;
  
  switch (obj->direction) {
    case UP: dy = -1; break;
    case DOWN: dy = 1; break;
    case LEFT: dx = -1; break;
    case RIGHT: dx = 1; break;
  }
  
  obj->x_px = obj->x_pos * CELL_WIDTH + dx * (obj->animation_frame * WALK_STEP_PX);
  obj->y_px = obj->y_pos * CELL_HEIGHT + dy * (obj->animation_frame * WALK_STEP_PX);
  
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
  
  if (cell_symbol == '=') {
    success |= make_object(ctx, OBJECT_WATER, x_pos, y_pos, &obj);
    success |= push_object_to_cell(obj, cell);
    obj->passable = 1;
  }
  else {
    success |= make_object(ctx, OBJECT_GRASS, x_pos, y_pos, &obj);
    success |= push_object_to_cell(obj, cell);
  }
  
  switch (cell_symbol) {
    case '#':
      success |= make_object(ctx, OBJECT_WALL, x_pos, y_pos, &obj);
      success |= push_object_to_cell(obj, cell);
      break;
    case '@':
      success |= make_object(ctx, OBJECT_HERO, x_pos, y_pos, &obj);
      success |= push_object_to_cell(obj, cell);
      break;
    case 'B':
      success |= make_object(ctx, OBJECT_BARREL, x_pos, y_pos, &obj);
      success |= push_object_to_cell(obj, cell);
      break;
    case 'h':
      success |= make_object(ctx, OBJECT_HUMAN, x_pos, y_pos, &obj);
      success |= push_object_to_cell(obj, cell);
      break;
    case 'D':
      success |= make_object(ctx, OBJECT_DOOR, x_pos, y_pos, &obj);
      success |= push_object_to_cell(obj, cell);
      break;
    case '4':
      success |= make_object(ctx, OBJECT_FOILHAT, x_pos, y_pos, &obj);
      success |= push_object_to_cell(obj, cell);
    case '~':
      success |= make_object(ctx, OBJECT_WATER, x_pos, y_pos, &obj);
      success |= push_object_to_cell(obj, cell);
      break;
    case '=':
      success |= make_object(ctx, OBJECT_BRIDGE, x_pos, y_pos, &obj);
      success |= push_object_to_cell(obj, cell);
      break;
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


int is_cell_visible(Cell* cell) {
  int visible = 1;
  for (int z = 0; z < cell->depth; ++z) {
    visible &= cell->objects[z]->visibility;
  }
  return visible;
}

