#pragma once
#include <pebble.h>
#define CHECKBOX_WINDOW_CELL_HEIGHT_DEP 44

static TextLayer *s_first_element;

//static void window_load(Window *window);
//static void window_unload(Window *window);
//static void generate_list(void);
void departure_window_push(char departures[256]);
//static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context);
//static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context);
//static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context);