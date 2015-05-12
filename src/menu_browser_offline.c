#include <pebble.h>
#include "menu_browser_offline.h"

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
  push_menu_offline(NULL, NULL, NULL);
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
  printf("browser: %p", s_menu_browsers[s_browser_index]);
  printf("titles: %p", s_menu_browsers[s_browser_index]->menu_titles);
  for (int i=0; i<4; i++)
  {
    printf("%s", s_menu_browsers[s_browser_index]->menu_titles[i]);
  }

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

  for (int i=0; i<4; i++)
  {
    free(browser->menu_titles[i]);
  }
  free(browser->menu_titles);

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
  }
  APP_LOG(APP_LOG_LEVEL_ERROR, "Unloaded");
}

void push_menu_offline(char *route, char *direction, char *stopid)
{ 
  if (s_menu_browsers == NULL)
  {
    printf("first push menu offline!");
    int num_browsers = 4;
    s_menu_browsers = calloc(num_browsers, sizeof(MenuBrowser*));
    s_browser_index = 0;
  }

  MenuBrowser *browser = calloc(1, sizeof(MenuBrowser)); // s_menu_browsers[s_browser_index];
  initialize_browser(browser);
  s_menu_browsers[s_browser_index] = browser;

  browser->menu_titles = calloc(4, sizeof(char *));
  for (int i=0; i<4; i++)
  {
    browser->menu_titles[i] = calloc(10, sizeof(char));
  }
  switch (s_browser_index)
  {
    case 0:
    {
      strcpy(browser->menu_titles[0], "zero");
      strcpy(browser->menu_titles[1], "one");
      strcpy(browser->menu_titles[2], "two");
      strcpy(browser->menu_titles[3], "three");
      break;
    }
    case 1:
    {
      strcpy(browser->menu_titles[0], "four");
      strcpy(browser->menu_titles[1], "five");
      strcpy(browser->menu_titles[2], "six");
      strcpy(browser->menu_titles[3], "seven");
      break;
    }
    case 2:
    {
      strcpy(browser->menu_titles[0], "eight");
      strcpy(browser->menu_titles[1], "nine");
      strcpy(browser->menu_titles[2], "ten");
      strcpy(browser->menu_titles[3], "eleven");
      break;
    }
    case 3:
    {
      strcpy(browser->menu_titles[0], "twelve");
      strcpy(browser->menu_titles[1], "thirteen");
      strcpy(browser->menu_titles[2], "fourteen");
      strcpy(browser->menu_titles[3], "fifteen");
      break;
    }
  }

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
}
