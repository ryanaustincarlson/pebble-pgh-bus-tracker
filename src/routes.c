#include <pebble.h>
#include "routes.h"
#include "str_split.h"
#include "app_constants.h"

/*
 * STATIC VARS
 */
  
static Window *s_routes_window;
static MenuLayer *s_menu_layer;

static char **s_menu_titles;
static char **s_menu_subtitles;
static int s_menu_num_entries; // total num items

/*
 * MENU
 */

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return 1;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
      return s_menu_num_entries;
    default:
      return 0;
  }
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  // Determine which section we're working with
  switch (section_index) {
    case 0:
      // Draw title text in the section header
      menu_cell_basic_header_draw(ctx, cell_layer, "Routes");
      break;
  }
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {  
  // Determine which section we're going to draw in
  switch (cell_index->section) {
    case 0:
    {
      // Use the row to specify which item we'll draw
      menu_cell_basic_draw(ctx,
        cell_layer,
        s_menu_titles[cell_index->row],
        s_menu_subtitles[cell_index->row],
        NULL);
      break;
    }
  }
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  // Use the row to specify which item will receive the select action
  switch (cell_index->row) {
    // Favorites
    case 0:
      break;
    
    // Routes
    case 1:
      
      break;
  }
}

/*
 * APP MESSAGES
 */

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Inbox Received");
  
  Tuple *t = dict_read_first(iterator);
  
  bool done = false;
  int item_index = -1;
  char *title;
  char *subtitle;
  
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
      case KEY_NUM_ENTRIES:
      {
        printf("num entries: %d", (int)t->value->int32);
        int num_entries = (int)t->value->int32;
        s_menu_titles = malloc(num_entries * sizeof(char *));
        s_menu_subtitles = malloc(num_entries * sizeof(char *));
        s_menu_num_entries = 0;
        break;
      }
      case KEY_ITEM_INDEX:
      {
        printf("item idx: %d", (int)t->value->int32);
        item_index = (int)t->value->int32;

        if (item_index > s_menu_num_entries)
        {
          s_menu_num_entries = item_index+1;
        }

        break;
      }
      case KEY_TITLES:
      {
        printf("titles (%d): %s", t->length, t->value->cstring);
        // char *tmptitle = t->value->cstring;
        // title = malloc(t->length * sizeof(char));
        // strcpy(title, tmptitle);
        // snprintf(title, sizeof(t->value->cstring), "%s", t->value->cstring);
        title = t->value->cstring;
        break;
      }
      case KEY_SUBTITLES:
        printf("subtitles (%d): %s", t->length, t->value->cstring);
        // char *tmpsubtitle = t->value->cstring;
        // subtitle = malloc(t->length * sizeof(char));
        // strcpy(subtitle, tmpsubtitle);
        subtitle = t->value->cstring;
        // snprintf(subtitle, sizeof(t->value->cstring), "%s", t->value->cstring);
        break;
    }
    
    if (strcmp("done", t->value->cstring) == 0)
    {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Done finding routes!");
      done = true;

      for (int i=0; i<s_menu_num_entries; i++)
      {
        char *atitle = s_menu_titles[i];
        char *asubtitle = s_menu_subtitles[i];
        printf("after > title: %s, subtitle: %s", atitle, asubtitle);
      }

      break;
    }
    
    // Look for next item
    t = dict_read_next(iterator);
  }

  if (item_index != -1)
  {
    printf("item idx valid = %d", item_index);
    printf("title: %s", title);
    printf("subtitle: %s", subtitle);

    // s_menu_titles[item_index] = malloc(strlen(title) * sizeof(char));

    s_menu_titles[item_index] = malloc(strlen(title) * sizeof(char));
    s_menu_subtitles[item_index] = malloc(strlen(subtitle) * sizeof(char));

    strcpy(s_menu_titles[item_index], title);
    strcpy(s_menu_subtitles[item_index], subtitle);

    // s_menu_titles[item_index] = title;
    // s_menu_subtitles[item_index] = subtitle;
    printf("title: %s, subtitle: %s", s_menu_titles[item_index], s_menu_subtitles[item_index]);
  }
  
  if (!done)
  {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_cstring(iter, 0, "getroutes");

    // Send the message!
    app_message_outbox_send();
    printf("sent getroutes message");
  }

  menu_layer_reload_data(s_menu_layer);
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

static void routes_window_load(Window *window) {
  // Now we prepare to initialize the menu layer
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  // Create the menu layer
  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = menu_get_header_height_callback,
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
  });

  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(s_menu_layer, window);

  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void routes_window_unload(Window *window) {
  // Destroy the menu layer
  menu_layer_destroy(s_menu_layer);
  
  // Destroy the window
  window_destroy(s_routes_window);
  
  APP_LOG(APP_LOG_LEVEL_ERROR, "Unloaded");
}

void push_routes(Window *window)
{ 
  // Create main Window element and assign to pointer
  s_routes_window = window_create();
  
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_routes_window, (WindowHandlers) {
    .load = routes_window_load,
    .unload = routes_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_routes_window, true);
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  // Add a key-value pair
  dict_write_cstring(iter, 0, "getroutes");

  // Send the message!
  app_message_outbox_send();
  // printf("sent getroutes message");
}
