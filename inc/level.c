#include "level.h"
#include "sdlike.h"


char* obj_type_to_str(ObjType type) {
  switch (type) {
    case GRASS: return "grass";
    case WALL: return "wall";
    default:
      dp("Unknown type for convering to string.\n");
      return "error";
  }
}


ObjType str_to_obj_type(char* str) {
  if (strcmp("grass", str) == 0) return GRASS;
  if (strcmp("wall", str) == 0) return WALL;
  return ERROR;
}


int make_object(
  ObjType type,
  TileStoreNode* tile_store,
  //
  Object** obj_ptr
) {
  dp("Make object.\n");
  *obj_ptr = (Object*)malloc(sizeof(Object));
  Object* obj = *obj_ptr;
  obj->type = type;
  
  Tile* tile;
  if (find_in_tile_store(tile_store, obj_type_to_str(type), &tile)) {
    return 1;
  }
  obj->tile = tile;
  
  return 0;
}


int make_cell_from_char(
  char cell_symbol,
  TileStoreNode* tile_store,
  //
  Cell** cell_ptr
) {
  int success = 0;
  *cell_ptr = (Cell*)malloc(sizeof(Cell));
  Cell* cell = *cell_ptr;
  cell->depth = 0;
  cell->capacity = MIN_CELL_CAPACITY;
  cell->objects = malloc(sizeof(Object*) * MIN_CELL_CAPACITY);

  Object* obj;
  success |= make_object(GRASS, tile_store, &obj);
  success |= push_object_to_cell(obj, cell);
  
  if (cell_symbol == '#') {
    success |= make_object(WALL, tile_store, &obj);
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
    cell->objects = malloc(sizeof(Object*) * cell->capacity);
    free(old_stack);
  }

  cell->objects[cell->depth] = obj;
  cell->depth += 1;
  return 0;
}


int make_level_from_file(
  char* path,
  TileStoreNode* tile_store,
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
  
  SDL_Surface* grass = NULL;
  if (load_image("./tiles/grass.png", &grass)) {
    goto error;
  }
  
  SDL_Surface* wall = NULL;
  if (load_image("./tiles/wall.png", &wall)) {
    goto error;
  }
  
  Cell* cell = NULL;
  
  // TODO: check malloc errors
  level->cells = (Cell***)malloc(sizeof(Cell**) * level->w);
  
  for (int x = 0; x < level->w; ++x) {
    level->cells[x] = (Cell**)malloc(sizeof(Cell*) * level->h);
    
    for (int y = 0; y < level->h; ++y) {
      char cell_symbol;
      if (fscanf(file, "%c", &cell_symbol) == EOF) {
        printf("Error on read pos %u;%u.\n", x, y);
        goto error;
      }
      dp("Read cell %u;%u symbol `%c`\n", x, y, cell_symbol);
      
      make_cell_from_char(cell_symbol, tile_store, &cell);
      level->cells[x][y] = cell;
    }
    fscanf(file, "\n");  // slurp NL
  }

  fclose(file);
  return 0;
  
error:
  fclose(file);
  return 1;
}


void print_level(Level* level) {
  for (int x = 0; x < level->w; ++x) {
    for (int y = 0; y < level->h; ++y) {
      for (int z = 0; z < level->cells[x][y]->depth; ++z) {
        unsigned depth = level->cells[x][y]->depth;
        
        if (depth > 0) {
          Object* obj = level->cells[x][y]->objects[depth - 1];
          
          switch (obj->type) {
            case GRASS: printf(" "); break;
            case WALL: printf("#"); break;
            default: dp("Unknown cell type `%u`.\n", obj->type);
          }
        } else {
          printf("0");
        }
      }
    }
    printf("\n");
  }
}


int place_level(Level* level, SDL_Surface* screen) {
  int success = 0;

  for (int x = 0; x < level->w; ++x) {
    for (int y = 0; y < level->h; ++y) {
      for (int z = 0; z < level->cells[x][y]->depth; ++z) {
        dp("Render cell %u;%u.\n", x, y);
        success |= place_surface_pos(x, y, level->cells[x][y]->objects[z]->tile->image, screen);
      }
    }
  }
  
  return success;
}

