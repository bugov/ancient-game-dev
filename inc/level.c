#include "level.h"


int make_level_from_file(
  char* path,
  //
  Level** level_ref
) {
  *level_ref = (Level*)malloc(sizeof(Level));
  Level* level = *level_ref;
  
  FILE* file = fopen(path, "r");
  unsigned w;
  unsigned h;
  
  if (fscanf(file, "%u %u\n", &w, &h) != 2) {
  #ifdef DEBUG
		printf("Can't read level size.\n");
  #endif
    goto error;
  }
  
  level->w = w;
  level->h = h;
  
  #ifdef DEBUG
		printf("Init level with size: %u;%u.\n", w, h);
  #endif
  
  SDL_Surface* grass = NULL;
  if (load_image("./tiles/grass.png", &grass)) {
    goto error;
  }
  
  SDL_Surface* wall = NULL;
  if (load_image("./tiles/wall.png", &wall)) {
    goto error;
  }
  
  // TODO: check malloc errors
  level->cells = (Cell***)malloc(sizeof(Cell**) * level->w);
  
  for (int x = 0; x < level->w; ++x) {
    level->cells[x] = (Cell**)malloc(sizeof(Cell*) * level->h);
    
    for (int y = 0; y < level->h; ++y) {
      level->cells[x][y] = (Cell*)malloc(sizeof(Cell));
  
      char cell_type;
  retry:
      cell_type = fgetc(file);
      
      #ifdef DEBUG
      printf(
        "Read cell %u;%u symbol `%c`\n",
        x,
        y,
        cell_type
      );
      #endif
      
      switch (cell_type) {
        case ' ':
          level->cells[x][y]->type = "grass";
          level->cells[x][y]->image = grass;
          break;
        case '#':
          level->cells[x][y]->type = "wall";
          level->cells[x][y]->image = wall;
          break;
        case EOF:
          printf("Incorrect level file.\n");
          goto error;
        default:
          goto retry;
      }
    }
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
      if (strcmp(level->cells[x][y]->type, "grass") == 0) {
        printf(" ");
      }
      else if (strcmp(level->cells[x][y]->type, "wall") == 0) {
        printf("#");
      }
      else {
        #ifdef DEBUG
        printf("Unknown cell type `%s`.\n", level->cells[x][y]->type);
        #endif
      }
    }
    printf("\n");
  }
}


int place_level(Level* level, SDL_Surface* screen) {
  int success = 0;

  for (int x = 0; x < level->w; ++x) {
    for (int y = 0; y < level->h; ++y) {
      #ifdef DEBUG
      printf("Render cell %u;%u.\n", x, y);
      #endif
      success |= place_surface_pos(x, y, level->cells[x][y]->image, screen);
    }
  }
  
  return success;
}

