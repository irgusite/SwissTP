#include <pebble.h>
#include "departure_view.h"
#define MAX_DEPARTURE_COUNT 10

static Window *s_main_window;
//static TextLayer *s_first_element;
static MenuLayer *s_menu_layer;
static char *departureList;
static char destination[MAX_DEPARTURE_COUNT][32];
static char line[MAX_DEPARTURE_COUNT][5];
static char passingtime[MAX_DEPARTURE_COUNT][6];

static void generate_list(){
  int j = 0;
  int k = -1;
  int startchar = 0;
  int i = 0;
  char buffer[16];
  //add memory clearing
  while(departureList[i]!='%' && k < MAX_DEPARTURE_COUNT){
    memset(&buffer, 0, sizeof(buffer));
    if(departureList[i] == ':'){
      if(j==0){
        strncpy(line[k],&departureList[startchar+1],i-startchar-1);
      }
      if(j==1){
        strncpy(destination[k],&departureList[startchar+2],i-startchar-2);
      }
      j++;
      startchar = i;
    }
    else if(departureList[i]==';'){
      if(j==2){
        strncpy(buffer,&departureList[startchar+1],i-startchar-1);
        int timestamp = atoi(buffer);
        time_t letime = timestamp;
        strftime(passingtime[k], sizeof(buffer), "%H:%M", localtime(&letime));
      }
      k++;
      j=0;
      startchar = i;
    }
    i++;
  }

}

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return MAX_DEPARTURE_COUNT;
}

static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  char sub[32];
  int index = cell_index->row;
  snprintf(sub, sizeof(sub), "%s -> %s", line[index], destination[index]);
  menu_cell_basic_draw(ctx, cell_layer, passingtime[cell_index->row], sub, NULL);
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  return PBL_IF_ROUND_ELSE(
    menu_layer_is_index_selected(menu_layer, cell_index) ?
      MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT : MENU_CELL_ROUND_UNFOCUSED_TALL_CELL_HEIGHT,
    CHECKBOX_WINDOW_CELL_HEIGHT_DEP);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  menu_layer_set_normal_colors(s_menu_layer, GColorBlack, GColorWhite);
  menu_layer_set_highlight_colors(s_menu_layer, GColorOrange, GColorWhite);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
      .get_num_rows = get_num_rows_callback,
      .draw_row = draw_row_callback,
      .get_cell_height = get_cell_height_callback,
      //.select_click = select_callback,
  });
  generate_list();
  APP_LOG(APP_LOG_LEVEL_INFO, "list generated");
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);

  window_destroy(window);
  s_main_window = NULL;
}


void departure_window_push(char *departures) {
  if(!s_main_window) {
    s_main_window = window_create();
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
  }
  departureList = departures;
  window_stack_push(s_main_window, true);
}