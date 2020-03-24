#include "game.h"


void handle_inventory_clicks(Context* ctx, int x_rel_px, int y_rel_px) {
  // Calc slots top-left corner
  int slot_x_pos = (x_rel_px + CELL_WIDTH / 2) / (CELL_WIDTH / 2);
  int slot_y_pos = (y_rel_px + CELL_HEIGHT * 2.5) / (CELL_HEIGHT / 2);
  dp("Click on inventory item: %d;%d\n", slot_x_pos, slot_y_pos);
  
  if ( // Not an inventory
      slot_x_pos <= -INVENTORY_WIDTH || slot_x_pos >= INVENTORY_WIDTH
      || slot_y_pos <= -1 || slot_y_pos >= INVENTORY_HEIGHT * 2 - 1
  ) {
    if (ctx->selected_slot != NULL) {
      drop_selected_slot(ctx, ctx->hero);
    }
    return;
  }
  
  if ( // Backpack slots
      slot_x_pos >= 0 && slot_x_pos < INV_SLOTS_WIDTH
      && slot_y_pos >= 0 && slot_y_pos < INV_SLOTS_HEIGHT
  ) {
    if ( // No item selected (let's select item)
      ctx->selected_slot == NULL
      || ctx->hero->slots[slot_x_pos][slot_y_pos]->obj_type != OBJECT_ERROR
    ) {
      ctx->selected_slot = ctx->hero->slots[slot_x_pos][slot_y_pos];
      ctx->selected_slot->animation_frame ^= 1;
      return;
    }
    
    if (ctx->selected_slot != NULL) {
      ctx->selected_slot->animation_frame ^= 1;
      swap_inventory_slots(ctx->hero, ctx->selected_slot, ctx->hero->slots[slot_x_pos][slot_y_pos]);
      ctx->selected_slot = NULL;
      return;
    }
  }
  
  // Try to fix slots' pos for equip
  slot_x_pos += 3;
  
  if ( // Equip slots
    slot_x_pos >= 0 && slot_x_pos < EQUIP_SLOTS_WIDTH
    && slot_y_pos >= 0 && slot_y_pos < EQUIP_SLOTS_HEIGHT
  ) {
    dp("Click on equip slot: %u;%u\n", slot_x_pos, slot_y_pos);
    if ( // No item selected (let's select item)
      ctx->selected_slot == NULL
      || ctx->hero->equip[slot_x_pos][slot_y_pos]->obj_type != OBJECT_ERROR
    ) {
      ctx->selected_slot = ctx->hero->equip[slot_x_pos][slot_y_pos];
      ctx->selected_slot->animation_frame ^= 1;
      return;
    }
    
    if (ctx->selected_slot != NULL) {
      ctx->selected_slot->animation_frame ^= 1;
      swap_inventory_slots(ctx->hero, ctx->selected_slot, ctx->hero->equip[slot_x_pos][slot_y_pos]);
      ctx->selected_slot = NULL;
      return;
    }
  }
}


void drop_selected_slot(Context* ctx, Object* obj) {
  if (ctx->selected_slot == NULL) return;
  
  Object* new_obj;
  make_object(ctx, ctx->selected_slot->obj_type, obj->x_pos, obj->y_pos, &new_obj);
  ctx->selected_slot->obj_type = OBJECT_ERROR;
  ctx->selected_slot = NULL;
  
  Cell* cell = ctx->level->cells[obj->x_pos][obj->y_pos];
  remove_object_from_cell(obj, cell);
  push_object_to_cell(new_obj, cell);
  push_object_to_cell(obj, cell);
}


int take_object_into_inventory(Context* ctx, Object* src, Object* dst) {
  if (! src->takeable) return 1;
  
  // find empty slot
  for (unsigned inv_x = 0; inv_x < INV_SLOTS_WIDTH; ++inv_x) {
    for (unsigned inv_y = 0; inv_y < INV_SLOTS_HEIGHT; ++inv_y) {
      if (dst->slots[inv_x][inv_y]->obj_type == OBJECT_ERROR) {
        dst->slots[inv_x][inv_y]->obj_type = src->type;
        dst->slots[inv_x][inv_y]->animation_frame = 0;
        dst->slots[inv_x][inv_y]->tile = src->tile;
        
        remove_object_from_cell(src, ctx->level->cells[src->x_pos][src->y_pos]);
        
        free_object(src);
        dp("Got an object and place to: %u;%u.\n", inv_x, inv_y);
        return 0;
      }
    }
  }
  dp("No free slots\n");
  return 2;
}


void swap_inventory_slots(Object* obj, Slot* a, Slot* b) {
  unsigned tmp_x = a->x_pos;
  unsigned tmp_y = a->y_pos;
  char is_equip = a->is_equip;
  
  a->x_pos = b->x_pos;
  a->y_pos = b->y_pos;
  a->is_equip = b->is_equip;
  b->x_pos = tmp_x;
  b->y_pos = tmp_y;
  b->is_equip = is_equip;
  
  if (b->is_equip) {
    obj->equip[b->x_pos][b->y_pos] = b;
  }
  else {
    obj->slots[b->x_pos][b->y_pos] = b;
  }
  
  if (a->is_equip) {
    obj->equip[a->x_pos][a->y_pos] = a;
  }
  else {
    obj->slots[a->x_pos][a->y_pos] = a;
  }
}


void render_inventory(Context* ctx) {
  for (int y = 0, tile_y = 0; y < INVENTORY_HEIGHT; ++y) {
    if (y > 0) tile_y = 1;
    if (y == INVENTORY_HEIGHT - 1) tile_y = 2;
    
    for (int x = 0, tile_x = 0; x < INVENTORY_WIDTH; ++x) {
      if (x > 0) tile_x = 1;
      if (x == INVENTORY_WIDTH - 1) tile_x = 2;
    
      // inventory interface
      render_surface_part_pos(
        1,
        1,
        x + ctx->hero->x_pos - INVENTORY_WIDTH / 2,
        y + ctx->hero->y_pos - INVENTORY_HEIGHT / 2,
        ctx->inventory_tile->image,
        ctx->renderer
      );
    
      render_surface_part_pos(
        tile_x,
        tile_y,
        x + ctx->hero->x_pos - INVENTORY_WIDTH / 2,
        y + ctx->hero->y_pos - INVENTORY_HEIGHT / 2,
        ctx->inventory_tile->image,
        ctx->renderer
      );
      
      // inventory slots borders
      if (x > 2 && y > 0) {
        render_surface_part_px(
          0,
          3 * CELL_HEIGHT,
          (x + ctx->hero->x_pos - INVENTORY_WIDTH / 2) * CELL_WIDTH - CELL_WIDTH / 2,
          (y + ctx->hero->y_pos - INVENTORY_HEIGHT / 2) * CELL_HEIGHT - CELL_HEIGHT / 2,
          ctx->inventory_tile->image,
          ctx->renderer
        );
      }
      
      // equip empty slots
      int equip_x, equip_y;
      if (x > 0 && x < 3 && y > 0 && y < 3) {
        if (x == 1 && y == 1) equip_x = 1, equip_y = 3;
        else if (x == 2 && y == 1) equip_x = 2, equip_y = 3;
        else if (x == 1 && y == 2) equip_x = 0, equip_y = 4;
        else if (x == 1 && y == 2) equip_x = 0, equip_y = 4;
        else if (x == 2 && y == 2) equip_x = 1, equip_y = 4;
        
        render_surface_part_px(
          equip_x * CELL_WIDTH,
          equip_y * CELL_HEIGHT,
          (x + ctx->hero->x_pos - INVENTORY_WIDTH / 2) * CELL_WIDTH - CELL_WIDTH / 2,
          (y + ctx->hero->y_pos - INVENTORY_HEIGHT / 2) * CELL_HEIGHT - CELL_HEIGHT / 2,
          ctx->inventory_tile->image,
          ctx->renderer
        );
      }
      
      // inventory slots items
      unsigned offset_x_ps = ctx->hero->x_px;
      unsigned offset_y_ps = (ctx->hero->y_pos - INVENTORY_HEIGHT / 2) * CELL_HEIGHT;
      
      for (unsigned inv_x = 0; inv_x < INV_SLOTS_WIDTH; ++inv_x) {
        for (unsigned inv_y = 0; inv_y < INV_SLOTS_HEIGHT; ++inv_y) {
          if (ctx->hero->slots[inv_x][inv_y]->obj_type != OBJECT_ERROR) {
            render_surface_part_px(
              ctx->hero->slots[inv_x][inv_y]->animation_frame * CELL_HEIGHT,
              1 * CELL_HEIGHT, // offset 1 for minimized icon (for inventory slot)
              offset_x_ps + (inv_x + 1) * CELL_WIDTH / 2,
              offset_y_ps + (inv_y + 1) * CELL_HEIGHT / 2,
              ctx->hero->slots[inv_x][inv_y]->tile->image,
              ctx->renderer
            );
          }
        }
      }
      
      // equip slots items
      for (unsigned equip_x = 0; equip_x < EQUIP_SLOTS_WIDTH; ++equip_x) {
        for (unsigned equip_y = 0; equip_y < EQUIP_SLOTS_HEIGHT; ++equip_y) {
          if (ctx->hero->equip[equip_x][equip_y]->obj_type != OBJECT_ERROR) {
            render_surface_part_px(
              ctx->hero->equip[equip_x][equip_y]->animation_frame * CELL_HEIGHT,
              1 * CELL_HEIGHT, // offset 1 for minimized icon (for inventory slot)
              offset_x_ps + (equip_x + 1) * CELL_WIDTH / 2 - CELL_WIDTH * 2,
              offset_y_ps + (equip_y + 1) * CELL_HEIGHT / 2,
              ctx->hero->equip[equip_x][equip_y]->tile->image,
              ctx->renderer
            );
          }
        }
      }
    }
  }
}

