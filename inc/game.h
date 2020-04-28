/**
  "Vegeta! What does the scouter say about his power level?"
*/

#ifndef _LEVEL_H_
#define _LEVEL_H_

#include "stdio.h"
#include "stdlib.h"
#include <SDL2/SDL.h>
#include "sdlike.h"

#define SEED 9293757

#define MAX_CELL_CAPACITY 256
#define MIN_CELL_CAPACITY 2
#define ATTACK_FRAMES 4
#define WALK_FRAMES 6
#define WALK_STEP_PX 16
#define INV_SLOTS_WIDTH 4
#define INV_SLOTS_HEIGHT 8
#define INVENTORY_WIDTH 5
#define INVENTORY_HEIGHT 5
#define EQUIP_SLOTS_WIDTH 3
#define EQUIP_SLOTS_HEIGHT 3
#define RERENDER_RANGE 8


typedef struct {
  char* text;
  SDL_Texture* image;
  unsigned w;
  unsigned h;
} Message;


typedef enum ObjType {
  OBJECT_ERROR,
  OBJECT_GRASS,
  OBJECT_WALL,
  OBJECT_DOOR,
  OBJECT_HERO,
  OBJECT_HUMAN,
  OBJECT_BARREL,
  OBJECT_FOILHAT,
  OBJECT_WATER,
  OBJECT_BRIDGE,
} ObjType;


typedef enum ObjState {
  STATE_ERROR,
  STATE_STAY,
  STATE_WALK,
  STATE_ATTACK,
} ObjState;


typedef enum ObjDirection {
  UP, LEFT, DOWN, RIGHT
} ObjDirection;


typedef struct Slot {
  unsigned x_pos;
  unsigned y_pos;
  
  char is_equip;
  
  ObjType obj_type;
  Tile* tile;
  unsigned animation_frame;
} Slot;


/**
  Any object on the Level
  (grass, wall, hero, etc)
*/
typedef struct Object {
  ObjType type;
  ObjState state;
  Tile* tile;
  Tile* walk_tile;
  Tile* attack_tile;
  
  int hp;
  unsigned base_attack;
  
  unsigned x_pos;
  unsigned y_pos;
  unsigned x_px;
  unsigned y_px;
  
  char can_walk;
  char passable;
  char walkable;
  char talkable;
  char takeable;
  char visibility;
  char base_visibility_range;
  
  char animation_frame;
  
  ObjDirection direction;
  
  Message** messages;
  
  Slot*** slots; // inventory
  Slot*** equip;
} Object;


/**
  Another cell on the field.
  Contains objects on this place.
*/
typedef struct Cell {
  unsigned depth;
  unsigned capacity;
  
  unsigned x_pos;
  unsigned y_pos;
  
  char changed;
  
  Object** objects;
} Cell;


/**
  World's level. Gaming area. 
  Field "cells" is two dimentional array of Cell.
*/
typedef struct Level {
  unsigned w;
  unsigned h;
  Cell*** cells;
} Level;


typedef enum GameMode {
  MODE_NORMAL,
  MODE_ATTACK,
  MODE_ANIMATION,
  MODE_TALK,
  MODE_INVENTORY
} GameMode;


typedef struct Context {
  GameMode mode;
  TileStoreNode* tile_store;
  Level* level;
  Object* hero;
  SDL_Renderer* renderer;
  
  unsigned window_width;
  unsigned window_height;
  
  char is_running;
  unsigned is_busy;
  
  Tile* inventory_tile;
  Slot* selected_slot;
} Context;


void handle_inventory_clicks(
  Context* ctx,
  int x_rel_px,
  int y_rel_px
);


int take_object_into_inventory(
  Context* ctx,
  Object* src,
  Object* dst
);


void swap_inventory_slots(
  Object* obj,
  Slot* a,
  Slot* b
);


void drop_selected_slot(
  Context* ctx,
  Object* obj
);


void render_inventory(Context* ctx);


int make_object(
  Context* ctx,
  ObjType type,
  unsigned x_pos,
  unsigned y_pos,
  //
  Object** obj_ptr
);

void free_object(Object* obj);

int set_walk_object_to_direction(
  Context* ctx,
  Object* obj,
  ObjDirection direction
);


int update_walk_frame(Context* ctx, Object* obj);


int make_cell_from_char(
  Context* ctx,
  char cell_symbol,
  unsigned x_pos,
  unsigned y_pos,
  //
  Cell** cell_ptr
);


int push_object_to_cell (
  Object* obj,
  Cell* cell
);


int remove_object_from_cell (
  Object* obj,
  Cell* cell
);


int is_cell_passable(Cell* cell);


int is_cell_visible(Cell* cell);


int make_level_from_file(
  Context* ctx,
  char* path,
  //
  Level** level_ptr
);


void fix_objects_tile(Context* ctx);


int set_attack_object(
  Context* ctx,
  Object* src,
  Object* dst
);


int update_attack_frame(
  Context* ctx,
  Object* obj
);


int render_level(Context* ctx);


void update_animation(Context* ctx);


int swap_object_between_cells(
  Object* obj,
  Cell* src,
  Cell* dst
);

#endif // _LEVEL_H_

