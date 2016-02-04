#include <pebble.h>
#include "departure_view.h"

#define CHECKBOX_WINDOW_CELL_HEIGHT 44
#define KEY_STATIONS 0
#define KEY_DEPARTURES 1
#define KEY_IDS 2
#define KEY_SEND_ID 3

static Window *s_main_window;
//static TextLayer *s_first_element;
static MenuLayer *s_menu_layer;
static char menubuffer[10][32];
static int s_menu_size;
static double s_station_id_list[10];
static bool sel_lock = 1;

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context){
  return s_menu_size;
}


static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  menu_cell_basic_draw(ctx, cell_layer, menubuffer[cell_index->row], NULL, NULL);
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  return PBL_IF_ROUND_ELSE(
    menu_layer_is_index_selected(menu_layer, cell_index) ?
      MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT : MENU_CELL_ROUND_UNFOCUSED_TALL_CELL_HEIGHT,
    CHECKBOX_WINDOW_CELL_HEIGHT);
}

static void send_int(int key, long value) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_int(iter, key, &value, sizeof(long), true);
  app_message_outbox_send();
}

static void show_departures(int line){
  //send app_message
  send_int(3,(long) s_station_id_list[line]);  
}

static void select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  if(!sel_lock){
    show_departures((int)cell_index->row);
  }
}

static void main_window_load(Window *window){
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  //create text layer with specific bounds
  s_first_element = text_layer_create(GRect(0, 0, bounds.size.w, 15));
  s_menu_layer = menu_layer_create(GRect(0, 15, bounds.size.w, 153));
  
  window_set_background_color(s_main_window, GColorBlack);
  
  menu_layer_set_click_config_onto_window(s_menu_layer, s_main_window);
  menu_layer_set_normal_colors(s_menu_layer, GColorBlack, GColorWhite);
  menu_layer_set_highlight_colors(s_menu_layer, GColorOrange, GColorWhite);
  menu_layer_set_callbacks(s_menu_layer, NULL,(MenuLayerCallbacks){
    .get_num_rows = get_num_rows_callback,
    .draw_row = draw_row_callback,
    .get_cell_height = get_cell_height_callback,
    .select_click = select_callback 
  });
  
  //first line
  text_layer_set_background_color(s_first_element, GColorBlack);
  text_layer_set_text_color(s_first_element, GColorWhite);
  text_layer_set_text(s_first_element, "00:00");
  text_layer_set_font(s_first_element, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text_alignment(s_first_element, GTextAlignmentCenter);

  layer_add_child(window_layer, text_layer_get_layer(s_first_element));
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
  
}
//window destruction
static void main_window_unload(Window *window){
  text_layer_destroy(s_first_element);
  menu_layer_destroy(s_menu_layer);
  
  //fonts_unload_custom_font(s_time_font);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  static char station_buffer[256];
  static char id_buffer[256];
  
  Tuple *station_tuple = dict_find(iterator, KEY_STATIONS);
  Tuple *id_tuple = dict_find(iterator, KEY_IDS);
  Tuple *departure_tuple = dict_find(iterator, KEY_DEPARTURES);
  
  if(station_tuple && id_tuple){
    snprintf(station_buffer, sizeof(station_buffer), "%s", station_tuple->value->cstring);
    snprintf(id_buffer, sizeof(id_buffer), "%s", id_tuple->value->cstring);
    APP_LOG(APP_LOG_LEVEL_INFO, "test");
    
    //creating the menu list
    int j = -1;
    u_long k = 0;
    for(u_long i=0; i < sizeof(station_buffer)/sizeof(char); i++){
      if(station_buffer[i] == ':'){
        j++;
        strncpy(menubuffer[j],&station_buffer[k+1],i-k-1);
        k = i+1;
      }
    }
    
    //creating the id list
    int j1 = -1;
    u_long k1 = 0;
    char id_int_buffer[10];
    for(u_long i=0; i < sizeof(id_buffer)/sizeof(char); i++){
      if(id_buffer[i] == ':'){
        j1++;
        strncpy(id_int_buffer,&id_buffer[k1],i-k1);
        s_station_id_list[j1] = atol(id_int_buffer);
        k1 = i+1;
      }
    }
    
    //defining the menu
    s_menu_size = j+1;
    menu_layer_reload_data(s_menu_layer);
    sel_lock = 0;
  }
  else if(departure_tuple){
    static char departure_buffer[350];
    snprintf(departure_buffer, sizeof(departure_buffer), "%s", departure_tuple->value->cstring);
    departure_window_push(departure_buffer);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
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

void handle_init(void) {
  s_main_window = window_create();
  
  s_first_element = text_layer_create(GRect(0, 0, 144, 20));
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload  
  });
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(500, 500);
  
  //get the time to display from the start
  update_time();
  
  //register to tickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  window_stack_push(s_main_window, true);
}

void handle_deinit(void) {
  window_destroy(s_main_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
