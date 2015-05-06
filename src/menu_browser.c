#include <pebble.h>
#include "menu_browser.h"

#include "app_constants.h"
#include "directions.h"

/*
 * STATIC VARS
 */

 typedef struct 
 {
  Window *menu_window;
  MenuLayer *menu_layer;
  char **menu_titles;
  char **menu_subtitles;
  int menu_num_entries;
} MenuBrowser;

// contains info for routes, direction, stops, predictions
// should be size >=4
static MenuBrowser **s_menu_browsers = NULL; // malloc(sizeof(MenuBrowser) * 4);
static int s_browser_index;

// static Window *s_menu_window;
// static MenuLayer *s_menu_layer;

/*
 * MENU
 */

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return 1;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
      return s_menu_browsers[s_browser_index]->menu_num_entries;
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
  char **titles = s_menu_browsers[s_browser_index]->menu_titles;
  char *title = titles[cell_index->row];
  
  // Determine which section we're going to draw in
  switch (cell_index->section) {
    case 0:
    {
      // Use the row to specify which item we'll draw
      menu_cell_basic_draw(ctx,
        cell_layer,
        title,
        NULL,
        NULL);
      break;
    }
  }
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  // Use the row to specify which item will receive the select action
  // char *title = s_menu_titles[cell_index->row];
  // push_directions(title);
  s_browser_index++;
  push_menu();
}

/*
 * APP MESSAGES
 */

 static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Inbox Received");

  MenuBrowser *browser = s_menu_browsers[s_browser_index];
  char **menu_titles = browser->menu_titles;
  printf("menu titles: %p, browser: %p", menu_titles, browser);
  
  Tuple *t = dict_read_first(iterator);
  
  bool done = false;
  int item_index = -1;
  char *title = NULL;
  char *subtitle = NULL;
  char *msg_type = NULL;
  
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
      case KEY_MSG_TYPE:
      {
        msg_type = t->value->cstring;
        printf("routes_ msg type: %s", msg_type);
        break;
      }
      case KEY_NUM_ENTRIES:
      {
        printf("routes_ num entries: %d", (int)t->value->int32);
        int num_entries = (int)t->value->int32;
        // s_menu_titles = malloc(num_entries * sizeof(char *));
        menu_titles = malloc(num_entries * sizeof(char *));
        browser->menu_titles = menu_titles;
        // s_menu_subtitles = malloc(num_entries * sizeof(char *));
        browser->menu_num_entries = 0;
        break;
      }
      case KEY_ITEM_INDEX:
      {
        printf("routes_ item idx: %d", (int)t->value->int32);
        item_index = (int)t->value->int32;

        break;
      }
      case KEY_TITLES:
      {
        printf("routes_ titles (%d): %s", t->length, t->value->cstring);
        title = t->value->cstring;
        break;
      }
      /*
      case KEY_SUBTITLES:
      {
        printf("routes_ subtitles (%d): %s", t->length, t->value->cstring);
        subtitle = t->value->cstring;
        break;
      }
      */
    }
    
    if (strcmp("done", t->value->cstring) == 0)
    {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Done finding routes!");
      done = true;
    }
    
    // Look for next item
    t = dict_read_next(iterator);
  }

  // if (strcmp("routes", msg_type) == 0)
  //{
  if (!done)
  {
    if (item_index != -1)
    {
    // printf("item idx valid = %d", item_index);
    // printf("title: %s", title);
    // printf("subtitle: %s", subtitle);

      if (item_index+1 > browser->menu_num_entries)
      {
        browser->menu_num_entries = item_index+1;
      }

      menu_titles[item_index] = malloc(strlen(title) * sizeof(char));
    // s_menu_titles[item_index] = malloc(strlen(title) * sizeof(char));
    // s_menu_subtitles[item_index] = malloc(strlen(subtitle) * sizeof(char));

      strcpy(menu_titles[item_index], title);
    // strcpy(s_menu_titles[item_index], title);
    // strcpy(s_menu_subtitles[item_index], subtitle);

    // printf("title: %s, subtitle: %s", s_menu_titles[item_index], s_menu_subtitles[item_index]);
      printf("local title: %s, title in array: %s", title, menu_titles[item_index]);
    }

    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

      // Add a key-value pair
    dict_write_cstring(iter, 0, "getroutes");

      // Send the message!
    app_message_outbox_send();
    printf("sent getroutes message");
  }

  // menu_layer_reload_data(s_menu_layer);
  menu_layer_reload_data(browser->menu_layer);
  //}
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

 /*
  * WINDOW MANAGEMENT
  */

static void window_load(Window *window) {
  // Now we prepare to initialize the menu layer
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  // Create the menu layer
  MenuLayer *menu_layer = menu_layer_create(bounds);
  s_menu_browsers[s_browser_index]->menu_layer = menu_layer;

  printf("menu_layer: %p", menu_layer);

  menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = menu_get_header_height_callback,
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
  });

  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(menu_layer, window);

  layer_add_child(window_layer, menu_layer_get_layer(menu_layer));
}

static void window_unload(Window *window) {
  MenuBrowser *browser = s_menu_browsers[s_browser_index];
  MenuLayer *menu_layer = browser->menu_layer;
  Window *menu_window = browser->menu_window;

  printf("menu_layer: %p", menu_layer);
  printf("menu_window: %p", menu_window);

  // Destroy the menu layer
  menu_layer_destroy(menu_layer);
  
  // Destroy the window
  window_destroy(menu_window);

  // clear out titles
  char **menu_titles = browser->menu_titles;
  for (int i=0; i<browser->menu_num_entries; i++)
  {
    free(menu_titles[i]);
  }
  free(menu_titles);
  browser->menu_titles = NULL;

  free(browser);
  
  APP_LOG(APP_LOG_LEVEL_ERROR, "Unloaded");

  if (s_browser_index > 0)
  {
    s_browser_index--;
  }
}

void push_menu()
{ 
  if (s_menu_browsers == NULL)
  {
    s_menu_browsers = malloc(sizeof(MenuBrowser*) * 4);
    s_browser_index = 0;
  }

  MenuBrowser *browser = malloc(sizeof(MenuBrowser));
  s_menu_browsers[s_browser_index] = browser;

  browser->menu_titles = NULL;
  browser->menu_num_entries = 0;

  // Create main Window element and assign to pointer
  Window *window = window_create();
  browser->menu_window = window;

  printf("menu_window1: %p", window);
  
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(window, true);

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  // Add a key-value pair
  dict_write_cstring(iter, 0, "getroutes");

  // Send the message!
  app_message_outbox_send();
  printf("sent getroutes message");
}
