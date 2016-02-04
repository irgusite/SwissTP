#include <pebble.h>
#include "departure_view.h"
#define MAX_DEPARTURE_COUNT 10

static Window *s_main_window;
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

static void update_time(){
  //get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style()? "%H:%M" : "%I:%M", tick_time);
  
  text_layer_set_text(s_first_element, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
  update_time();
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_menu_layer = menu_layer_create(GRect(0, 15, bounds.size.w, 153));
  s_first_element = text_layer_create(GRect(0, 0, bounds.size.w, 15));
  
  window_set_background_color(s_main_window, GColorBlack);
  
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  menu_layer_set_normal_colors(s_menu_layer, GColorBlack, GColorWhite);
  menu_layer_set_highlight_colors(s_menu_layer, GColorOrange, GColorWhite);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
      .get_num_rows = get_num_rows_callback,
      .draw_row = draw_row_callback,
      .get_cell_height = get_cell_height_callback,
      //.select_click = select_callback,
  });
  
  //first line
  text_layer_set_background_color(s_first_element, GColorBlack);
  text_layer_set_text_color(s_first_element, GColorWhite);
  //text_layer_set_text(s_first_element, "00:00");
  text_layer_set_font(s_first_element, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text_alignment(s_first_element, GTextAlignmentCenter);

  layer_add_child(window_layer, text_layer_get_layer(s_first_element));
  
  update_time();
  
  //register to tickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  generate_list();
  APP_LOG(APP_LOG_LEVEL_INFO, "list generated");
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);
  text_layer_destroy(s_first_element);

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