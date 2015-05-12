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
static char *MESSAGES[4] = {"getroutes", "getdirections", "getstops", "getpredictions"};
// static char *MESSAGES[4] = {"getroutes", "getdirections", "getstops", "getroutes"};

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



  MenuBrowser *browser = s_menu_browsers[s_browser_index];
  char *route = browser->route;
  char *direction = browser->direction;
  char *stopid = browser->stopid;

  switch (s_browser_index)
  {
    case 0:
    {
      // route = browser->menu_selectors[cell_index->row];
      route = browser->menu_titles[cell_index->row];
      break;
    }
    case 1:
    {
      // direction = browser->menu_selectors[cell_index->row];
      direction = browser->menu_titles[cell_index->row];
      break;
    }
    case 2:
    {
      stopid = browser->menu_selectors[cell_index->row];
      break;
    }
  }

  printf("pushing menu with route: %s (%p), direction: %s (%p), stopid: %s (%p)",
    route, route, direction, direction, stopid, stopid);

  s_browser_index++;
  push_menu(route, direction, stopid);
}

/*
 * APP MESSAGES
 */

 static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // APP_LOG(APP_LOG_LEVEL_ERROR, "Inbox Received");

  MenuBrowser *browser = s_menu_browsers[s_browser_index];
  char **menu_titles = browser->menu_titles;
  char **menu_selectors = browser->menu_selectors;
  // printf("menu titles: %p, browser: %p", menu_titles, browser);
  
  Tuple *t = dict_read_first(iterator);
  
  bool done = false;
  int item_index = -1;
  char *title = NULL;
  char *subtitle = NULL;
  char *selector = NULL;
  // char *msg_type = NULL;
  
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
      case KEY_MSG_TYPE:
      {
        // msg_type = t->value->cstring;
        // printf("msg type: %s", msg_type);
        break;
      }
      case KEY_NUM_ENTRIES:
      {
        // printf("num entries: %d", (int)t->value->int32);
        int num_entries = (int)t->value->int32;
        // s_menu_titles = malloc(num_entries * sizeof(char *));
        menu_titles = malloc(num_entries * sizeof(char *));
        browser->menu_titles = menu_titles;
        // s_menu_subtitles = malloc(num_entries * sizeof(char *));

        // menu_selectors = malloc(num_entries * sizeof(char *));
        // browser->menu_selectors = menu_selectors;

        browser->menu_num_entries = 0;
        break;
      }
      case KEY_ITEM_INDEX:
      {
        // printf("item idx: %d", (int)t->value->int32);
        item_index = (int)t->value->int32;

        break;
      }
      case KEY_TITLES:
      {
        // printf("titles (%d): %s", t->length, t->value->cstring);
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
      case KEY_SELECTORS:
      {
        // printf("selector (%d): %s", t->length, t->value->cstring);
        // selector = t->value->cstring;
        break;
      }
    }
    
    if (strcmp("done", t->value->cstring) == 0)
    {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Done finding messages!");
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

      // menu_selectors[item_index] = malloc(strlen(selector) * sizeof(char));
      // strcpy(menu_selectors[item_index], selector);

    // printf("title: %s, subtitle: %s", s_menu_titles[item_index], s_menu_subtitles[item_index]);
      // printf("local title: %s, title in array: %s", title, menu_titles[item_index]);
    }

    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    char *msg = MESSAGES[s_browser_index];
    dict_write_cstring(iter, 0, msg);

    // Send the message!
    app_message_outbox_send();
    // printf("sent %s message", msg);
  }
  else
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
  }

  // menu_layer_reload_data(s_menu_layer);
  // menu_layer_reload_data(browser->menu_layer);
  //}
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  // APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  // APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  // APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

 /*
  * WINDOW MANAGEMENT
  */

static void initialize_browser(MenuBrowser *browser)
{
  browser->menu_window = NULL;
  browser->menu_layer = NULL;

  browser->menu_titles = NULL;
  browser->menu_selectors = NULL;
  browser->menu_num_entries = 0;

  browser->route = NULL;
  browser->direction = NULL;
  browser->stopid = NULL;
}

static void window_load(Window *window) {
  // Now we prepare to initialize the menu layer
  // Layer *window_layer = window_get_root_layer(window);
  // GRect bounds = layer_get_frame(window_layer);

  // // Create the menu layer
  // MenuLayer *menu_layer = menu_layer_create(bounds);
  // s_menu_browsers[s_browser_index]->menu_layer = menu_layer;

  // printf("menu_layer: %p", menu_layer);

  // menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks){
  //   .get_num_sections = menu_get_num_sections_callback,
  //   .get_num_rows = menu_get_num_rows_callback,
  //   .get_header_height = menu_get_header_height_callback,
  //   .draw_header = menu_draw_header_callback,
  //   .draw_row = menu_draw_row_callback,
  //   .select_click = menu_select_callback,
  // });

  // Bind the menu layer's click config provider to the window for interactivity
  // menu_layer_set_click_config_onto_window(menu_layer, window);

  // layer_add_child(window_layer, menu_layer_get_layer(menu_layer));
}

static void window_unload(Window *window) {
  MenuBrowser *browser = s_menu_browsers[s_browser_index];
  MenuLayer *menu_layer = browser->menu_layer;
  Window *menu_window = browser->menu_window;

  printf("freeing menu_layer: %p", menu_layer);
  printf("freeing menu_window: %p", menu_window);

  // Destroy the menu layer
  if (menu_layer != NULL)
  {
    menu_layer_destroy(menu_layer);
    browser->menu_layer = NULL;
  }
  
  // Destroy the window
  if (menu_window != NULL)
  {
    window_destroy(menu_window);
    browser->menu_window = NULL;
  }

  // initialize_browser(browser);

  // clear out titles
  for (int i=0; i<browser->menu_num_entries; i++)
  {
    free(browser->menu_titles[i]);
    // free(browser->menu_selectors[i]);
  }
  free(browser->menu_titles);
  // free(browser->menu_selectors);
  browser->menu_titles = NULL;

  browser->menu_num_entries = 0;

  printf("could free route: %p", browser->route);
  if (browser->route != NULL)
  {
    printf("freeing route: %p, %s", browser->route, browser->route);
    free(browser->route);
  }
  browser->route = NULL;

  printf("could free dir: %p", browser->direction);
  if (browser->direction != NULL)
  {
    printf("freeing dir: %p", browser->direction);
    // free(browser->direction);
  }
  browser->direction = NULL;
  
  printf("could free stopid: %p", browser->stopid);
  if (browser->stopid != NULL)
  {  
    printf("freeing stopid: %p", browser->stopid);
    // free(browser->stopid);  
  }
  browser->stopid = NULL;

  printf("freeing browser: %p", browser);
  // free(browser);
  // free(s_menu_browsers[s_browser_index]);
  // s_menu_browsers[s_browser_index] = NULL;
  
  printf("decrementing browser index");
  if (s_browser_index > 0)
  {
    s_browser_index--;
  }
  APP_LOG(APP_LOG_LEVEL_ERROR, "Unloaded");
}

void push_menu(char *route, char *direction, char *stopid)
{ 
  if (s_menu_browsers == NULL)
  {
    int num_browsers = 4;
    s_menu_browsers = malloc(sizeof(MenuBrowser*) * num_browsers);
    s_browser_index = 0;

    for (int i=0; i<num_browsers; i++)
    {
      s_menu_browsers[i] = malloc(sizeof(MenuBrowser));
      // s_menu_browsers[i] = NULL;
    }
  }

  MenuBrowser *browser = s_menu_browsers[s_browser_index];
  // MenuBrowser *browser = malloc(sizeof(MenuBrowser));
  // s_menu_browsers[s_browser_index] = browser;
  printf("created browser: %p", browser);
  initialize_browser(browser);

  for (int i=0; i<4; i++)
  {
    printf("menu_browsers[%d] = %p", i, s_menu_browsers[i]);
  }

  if (route != NULL)
  {
    printf("copying route");
    browser->route = malloc(strlen(route) * sizeof(char));
    strcpy(browser->route, route);
  }

  if (direction != NULL)
  {
    // printf("copying direction");
    browser->direction = malloc(strlen(direction) * sizeof(char));
    strcpy(browser->direction, direction);
  }

  if (stopid != NULL)
  {
    // printf("copying stopid");
    browser->stopid = malloc(strlen(stopid) * sizeof(char));
    strcpy(browser->stopid, stopid);
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

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  // Add a key-value pair
  char *msg = MESSAGES[s_browser_index];
  dict_write_cstring(iter, 0, msg);

  if (route != NULL)
    dict_write_cstring(iter, 1, route);
  if (direction != NULL)
    dict_write_cstring(iter, 2, direction);
  if (stopid != NULL)
    dict_write_cstring(iter, 3, stopid);

  // Send the message!
  app_message_outbox_send();
  printf("sent %s message", msg);
}
