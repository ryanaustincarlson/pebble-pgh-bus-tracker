#include <pebble.h>
#include "menu_browser_offline.h"

#include "app_constants.h"
#include "directions.h"
#include "str_utils.h"

/*
 * STATIC VARS
 */

 typedef struct 
 {
  Window *menu_window;
  MenuLayer *menu_layer;
  char **menu_titles;
  char **menu_subtitles;
  char **menu_selectors; // what to send back to the phone
  int menu_num_entries;
  char *route;
  char *direction;
  char *stopid;
} MenuBrowser;

// contains info for routes, direction, stops, predictions
// should be size >=4
static MenuBrowser **s_menu_browsers = NULL; // malloc(sizeof(MenuBrowser) * 4);
static int s_browser_index;

static char *HEADERS[4] = {"Routes", "Directions", "Stops", "Predictions"};
// static char *MESSAGES[4] = {"getroutes", "getdirections", "getstops", "getpredictions"};
static char *MESSAGES[4] = {"getroutes", "getdir", "getstops", "getroutes"};

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
      // return s_menu_browsers[s_browser_index]->menu_num_entries;
      return 4;
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
    {
      char *header = HEADERS[s_browser_index];
      if (header != NULL)
      {
        menu_cell_basic_header_draw(ctx, cell_layer, header);
      }
      break;
    }
  }
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  char *title = s_menu_browsers[s_browser_index]->menu_titles[cell_index->row];
  /*char *title = NULL;
  if (s_browser_index == 0)
  {
    switch (cell_index->row)
    {
      case 0: { title = "zero"; break; }
      case 1: { title = "one"; break; }
      case 2: { title = "two"; break; }
      case 3: { title = "three"; break; }
    }
  }
  else if (s_browser_index == 1)
  {
    switch (cell_index->row)
    {
      case 0: { title = "four"; break; }
      case 1: { title = "five"; break; }
      case 2: { title = "six"; break; }
      case 3: { title = "seven"; break; }
    }
  }
  else if (s_browser_index == 2)
  {
    switch (cell_index->row)
    {
      case 0: { title = "eight"; break; }
      case 1: { title = "nine"; break; }
      case 2: { title = "ten"; break; }
      case 3: { title = "eleven"; break; }
    }
  }
  else
  {
    switch (cell_index->row)
    {
      case 0: { title = "twelve"; break; }
      case 1: { title = "thirteen"; break; }
      case 2: { title = "fourteen"; break; }
      case 3: { title = "fifteen"; break; }
    }
  }*/
  
  printf("title: %s", title);

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
  MenuBrowser *browser = s_menu_browsers[s_browser_index];
  char *route = browser->route;
  char *direction = browser->direction;
  char *stopid = browser->stopid;
  
  char *selector = browser->menu_titles[cell_index->row];
  if (s_browser_index == 0)
  {
    route = selector;
  }
  else if (s_browser_index == 1)
  {
    direction = selector;
  }
  else if (s_browser_index == 2)
  {
    stopid = selector;
  }

  
  // char *route = "P1";
  // char *direction = "INBOUND";
  // char *stopid = NULL;

  printf("pushing menu with route: %s (%p), direction: %s (%p), stopid: %s (%p)",
    route, route, direction, direction, stopid, stopid);

  s_browser_index++;
  push_menu_offline(route, direction, stopid);
}

/*
 * APP MESSAGES
 */

static void inbox_received_callback(DictionaryIterator *iterator, void *context) 
{
  // APP_LOG(APP_LOG_LEVEL_ERROR, "Inbox Received");

  MenuBrowser *browser = s_menu_browsers[s_browser_index];

  bool done = false;
  int item_index = -1;
  char *title = NULL;
  char *subtitle = NULL;
  char *selector = NULL;

  Tuple *t = dict_read_first(iterator);
  while (t != NULL)
  {
    switch(t->key)
    {
      case KEY_NUM_ENTRIES:
      {
        int num_entries = (int)t->value->int32;
        printf("num entries: %d", num_entries);
        browser->menu_titles = calloc(num_entries, sizeof(char *));
        break;
      }
      case KEY_ITEM_INDEX:
      {
        item_index = (int)t->value->int32;
        break;
      }
      case KEY_TITLES:
      {
        title = t->value->cstring;
        break;
      }
      case KEY_SUBTITLES:
      {
        subtitle = t->value->cstring;
        break;
      }
      case KEY_SELECTORS:
      {
        selector = t->value->cstring;
        break;
      }
    }

    if (strcmp("done", t->value->cstring) == 0)
    {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Done finding messages!");
      done = true;
    }

    t = dict_read_next(iterator);
  }

  if (done)
  {
    Window *window = browser->menu_window;
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

    menu_layer_set_click_config_onto_window(menu_layer, window);
    layer_add_child(window_layer, menu_layer_get_layer(menu_layer));
    menu_layer_reload_data(menu_layer);
    // menu_layer_reload_data(browser->menu_layer);
  }
  else
  {
    
    if (item_index != -1)
    {
      if (item_index+1 > browser->menu_num_entries)
      {
        browser->menu_num_entries = item_index+1;
      }

      printf("title: %s, length: %d", title, strlen(title));
      browser->menu_titles[item_index] = strdup(title);

      printf("idx: %d, title: %s, subt: %s, sel: %s", item_index, 
        browser->menu_titles[item_index], subtitle, selector);
    }


    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    char *msg = MESSAGES[s_browser_index];
    dict_write_cstring(iter, 0, msg);

    // Send the message!
    app_message_outbox_send();
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

 /*
  * WINDOW MANAGEMENT
  */

static void initialize_browser(MenuBrowser *browser)
{
  browser->menu_window = NULL;
  browser->menu_layer = NULL;

  browser->menu_titles = NULL;
  browser->menu_subtitles = NULL;
  browser->menu_selectors = NULL;
  browser->menu_num_entries = 0;

  browser->route = NULL;
  browser->direction = NULL;
  browser->stopid = NULL;
}

static void window_load(Window *window) {
  /*
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
  menu_layer_reload_data(menu_layer);*/
  
}

static void window_unload(Window *window) {
  MenuBrowser *browser = s_menu_browsers[s_browser_index];
  MenuLayer *menu_layer = browser->menu_layer;
  Window *menu_window = browser->menu_window;

  printf("freeing menu_layer: %p", menu_layer);
  // Destroy the menu layer
  if (menu_layer)
    menu_layer_destroy(menu_layer);
  browser->menu_layer = NULL;
  
  printf("freeing menu_window: %p", menu_window);
  // Destroy the window
  if (menu_window)
    window_destroy(menu_window);
  browser->menu_window = NULL;

  printf("num entries: %d", browser->menu_num_entries);
  for (int i=0; i<browser->menu_num_entries; i++)
  {
    printf("i : %d", i);
    printf("freeing %s (%p), len: %d", 
      browser->menu_titles[i], browser->menu_titles[i], strlen(browser->menu_titles[i]));
    free(browser->menu_titles[i]);
  }
  printf("freeing menu titles array %p", browser->menu_titles);
  free(browser->menu_titles);
  browser->menu_num_entries = 0;

  if (browser->route)
  {
    printf("freeing route %p", browser->route);
    free(browser->route);
    printf("done freeing route");
  }

  if (browser->direction)
  {
    printf("freeing direction: %p", browser->direction);
    free(browser->direction);
    printf("done freeing direction");
  }

  if (browser->stopid)
  {
    printf("freeing stopid: %p", browser->stopid);
    free(browser->stopid);
    printf("done freeing stopid");
  }

  printf("freeing browser: %p", s_menu_browsers[s_browser_index]);
  free(browser);
  s_menu_browsers[s_browser_index] = NULL;
  
  printf("decrementing browser index");
  if (s_browser_index > 0)
  {
    s_browser_index--;
  }
  else
  {
    free(s_menu_browsers);
    s_menu_browsers = NULL;
    app_message_deregister_callbacks();
  }

  APP_LOG(APP_LOG_LEVEL_ERROR, "Unloaded");
}

void push_menu_offline(char *route, char *direction, char *stopid)
{ 
  if (s_menu_browsers == NULL)
  {
    printf("first push menu offline!");
    int num_browsers = 4;
    s_menu_browsers = calloc(num_browsers, sizeof(MenuBrowser *));
    s_browser_index = 0;

    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);
  }

  MenuBrowser *browser = calloc(1, sizeof(MenuBrowser)); // s_menu_browsers[s_browser_index];
  initialize_browser(browser);
  s_menu_browsers[s_browser_index] = browser;

  printf("created browser: %p", browser);

  for (int i=0; i<4; i++)
  {
    printf("menu_browsers[%d] = %p", i, s_menu_browsers[i]);
  }

  // Create main Window element and assign to pointer
  Window *window = window_create();
  browser->menu_window = window;

  printf("created menu_window: %p", window);
  
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(window, true);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  // Add a key-value pair
  char *msg = MESSAGES[s_browser_index];
  dict_write_cstring(iter, 0, msg);

  if (route != NULL)
  {
    browser->route = strdup(route);
    dict_write_cstring(iter, 1, route);
  }
  if (direction != NULL)
  {
    browser->direction = strdup(direction);
    dict_write_cstring(iter, 2, direction);
  }
  if (stopid != NULL)
  {
    browser->stopid = strdup(stopid);
    dict_write_cstring(iter, 3, stopid);
  }

  // Send the message!
  app_message_outbox_send();
  printf("sent %s message", msg);
}
