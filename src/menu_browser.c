#include "menu_browser.h"
#include "str_utils.h"

#include "app_constants.h"
#include "app_colors.h"
#include "app_message_utils.h"

#include "settings_actionbar.h"
#include "shared_ui.h"

/*
 * CONSTANTS / STATIC VARS
 */

#define MIN_ROW_HEIGHT 44
#define MAX_ROW_HEIGHT (int)MIN_ROW_HEIGHT * 2.5

// contains info for routes, direction, stops, predictions
static MenuBrowser **s_menu_browsers = NULL;
static int s_browser_index;

static TextLayer *s_text_layer_noresults = NULL;

// it'd be nice if these are all in a struct or something...
static AppTimer *s_timer = NULL; // timer that calls a fcn to send the first app msg
static bool s_timer_fired = false; // true if the timer has already fired
static int s_timer_browser_index = -1; // the browser index that we expect when the timer fires

/*
 * Utilty Functions
 */
unsigned long get_timestamp()
{
  time_t seconds;
  uint16_t millis;
  time_ms(&seconds, &millis);
  unsigned long timestamp = seconds * 1000 + millis; // report milliseconds
  return timestamp;
}

int get_text_height(char *text, const Layer *layer, const char *font_key)
{
  if (!text || !layer)
  {
    return 0;
  }
  GRect bounds = layer_get_frame(layer);
  int cell_width = bounds.size.w - 10;
  GSize size = graphics_text_layout_get_content_size(
    text,
    fonts_get_system_font(font_key),
    GRect(5, 0, cell_width, MAX_ROW_HEIGHT),
    GTextOverflowModeWordWrap,
    GTextAlignmentLeft);
  return size.h;
}

/*
 * Text Layers
 */

void setup_text_layer_noresults(Window *window)
{
  if (!s_text_layer_noresults)
  {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_frame(window_layer);

    s_text_layer_noresults = text_layer_create(bounds);
    text_layer_set_font(s_text_layer_noresults, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_text_alignment(s_text_layer_noresults, GTextAlignmentCenter);
    #ifdef PBL_COLOR
      // text_layer_set_text_color(s_text_layer_noresults, get_color_error());
      text_layer_set_background_color(s_text_layer_noresults, get_color_error());
    #endif
  }

  MenuBrowser *browser = s_menu_browsers[s_browser_index];
  char *keyword = NULL;
  if (strcmp(browser->msg, MSG_ROUTES) == 0)
  {
    keyword = "routes";
  }
  else if (strcmp(browser->msg, MSG_DIRECTIONS) == 0)
  {
    keyword = "directions";
  }
  else if (strcmp(browser->msg, MSG_STOPS) == 0)
  {
    keyword = "stops";
  }
  else if (strcmp(browser->msg, MSG_PREDICTIONS) == 0)
  {
    keyword = "predictions";
  }
  else if (strcmp(browser->msg, MSG_FAVORITES) == 0)
  {
    keyword = "favorites";
  }
  else if (strcmp(browser->msg, MSG_NEARBY_STOPS) == 0)
  {
    keyword = "nearby stops";
  }
  else if (strcmp(browser->msg, MSG_NEARBY_ROUTES) == 0)
  {
    keyword = "nearby routes";
  }
  else if (strcmp(browser->msg, MSG_COMMUTE) == 0)
  {
    keyword = "commute routes";
  }

  char *start = "No ";
  char *end = "\nto display.";
  char *message = calloc(strlen(start) + strlen(keyword) + strlen(end) + 1, sizeof(char));
  strcat(message, start);
  strcat(message, keyword);
  strcat(message, end);
  text_layer_set_text(s_text_layer_noresults, message);
  free(message);
}

void send_menu_app_message_helper(void *data)
{
  s_timer_fired = true;
  printf("in menu app msg helper");
  printf("browser index...");
  printf("%d", s_browser_index);
  if (s_timer_browser_index == s_browser_index)
  {
    printf("timer browser idx: %d, browser_index: %d", s_timer_browser_index, s_browser_index);
    printf("sending menu app msg");
    send_menu_app_message(s_menu_browsers[s_browser_index]);
  }
}

/*
 * MENU
 */

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data)
{
  return 1;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data)
{
  MenuBrowser *browser = s_menu_browsers[s_browser_index];
  int num_entries = browser->menu_num_entries;

  return num_entries;
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data)
{
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static int16_t menu_get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data)
{
  MenuBrowser *browser = s_menu_browsers[s_browser_index];

  int title_height = browser->menu_title_heights[cell_index->row];
  int subtitle_height = browser->menu_subtitle_heights[cell_index->row];
  return  title_height + (subtitle_height + 1) + 8;
}

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data)
{
  MenuBrowser *browser = s_menu_browsers[s_browser_index];
  char *msg = browser->msg;

  char *header = NULL;
  if (strcmp(msg, MSG_ROUTES) == 0)
  {
    header = "Routes";
  }
  else if (strcmp(msg, MSG_DIRECTIONS) == 0)
  {
    header = "Directions";
  }
  else if (strcmp(msg, MSG_STOPS) == 0)
  {
    header = "Stops";
  }
  else if (strcmp(msg, MSG_PREDICTIONS) == 0)
  {
    header = "Predictions";
  }
  else if (strcmp(msg, MSG_FAVORITES) == 0)
  {
    header = "Favorites";
  }
  else if (strcmp(msg, MSG_NEARBY_STOPS) == 0)
  {
    header = "Nearby Stops";
  }
  else if (strcmp(msg, MSG_NEARBY_ROUTES) == 0)
  {
    header = "Nearby Routes";
  }
  else if (strcmp(msg, MSG_COMMUTE) == 0)
  {
    header = "Commute";
  }

  menu_cell_basic_header_draw(ctx, cell_layer, header);
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data)
{
  MenuBrowser *browser = s_menu_browsers[s_browser_index];

  char *title = browser->menu_titles[cell_index->row];
  char *subtitle = browser->menu_subtitles[cell_index->row];

  GRect bounds = layer_get_frame(cell_layer);

  int title_height = browser->menu_title_heights[cell_index->row];
  int subtitle_height = browser->menu_subtitle_heights[cell_index->row];
  int cell_width = bounds.size.w - 10;

  graphics_draw_text(ctx, title,
    fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(5, 0, cell_width, title_height),
    GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

  graphics_draw_text(ctx, subtitle,
    fonts_get_system_font(FONT_KEY_GOTHIC_24), GRect(5, title_height + 1, cell_width, subtitle_height),
    GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data)
{
  MenuBrowser *browser = s_menu_browsers[s_browser_index];

  if (browser->menu_num_entries == 0)
  {
    return;
  }

  menu_layer_reload_data(browser->menu_layer);

  char *msg = browser->msg;
  char *route = browser->route;
  char *direction = browser->direction;
  char *stopid = browser->stopid;
  char *stopname = browser->stopname;
  char *extra = browser->extra;

  char *selector = browser->menu_selectors[cell_index->row];
  char *new_msg = NULL;
  char **split = NULL; // in case we need to split things... need to free afterward
  // printf("starting strcmps w/ msg: %s", msg ? msg : "NULL");

  if (strcmp(msg, MSG_ROUTES) == 0)
  {
    new_msg = MSG_DIRECTIONS;
    route = selector;
  }
  else if (strcmp(msg, MSG_DIRECTIONS) == 0)
  {
    new_msg = MSG_STOPS;
    direction = selector;
  }
  else if (strcmp(msg, MSG_STOPS) == 0)
  {
    new_msg = MSG_PREDICTIONS;
    stopid = selector;
    stopname = browser->menu_titles[cell_index->row];
  }
  else if (strcmp(msg, MSG_FAVORITES) == 0)
  {
    new_msg = MSG_PREDICTIONS;
    extra = selector;
  }
  else if (strcmp(msg, MSG_NEARBY_STOPS) == 0)
  {
    new_msg = MSG_PREDICTIONS;
    stopid = selector;
  }
  else if (strcmp(msg, MSG_NEARBY_ROUTES) == 0)
  {
    new_msg = MSG_PREDICTIONS;

    split = str_split(selector, '_');
    if (split != NULL)
    {
      route = split[0];
      stopname = split[1];
      direction = split[2];
    }
  }
  else if (strcmp(msg, MSG_PREDICTIONS) == 0 || strcmp(msg, MSG_COMMUTE) == 0)
  {
    char *selector_copy = strdup(selector);
    split = str_split(selector_copy, '_');
    free(selector_copy);
    if (split != NULL)
    {
      route = split[0];
      direction = split[1];
      stopid = split[2];
      stopname = split[3];
    }
  }

  if (new_msg)
  {
    printf("msg: %s, route: %s, direction: %s, stopid: %s, stopname: %s: extra: %s",
    msg ? msg : "NULL",
    route ? route : "NULL",
    direction ? direction : "NULL",
    stopid ? stopid : "NULL",
    stopname ? stopname : "NULL",
    extra ? extra : "NULL");

    push_menu(new_msg, route, direction, stopid, stopname, extra);
  }
  else if (split != NULL)
  {
    push_settings_actionbar(route, direction, stopid, stopname);
  }

  if (split != NULL)
  {
    for (int i=0; split[i] != NULL; i++)
    {
      free(split[i]);
    }
    free(split);
  }
}

/*
* APP MESSAGES
*/

static void inbox_received_callback(DictionaryIterator *iterator, void *context)
{
  APP_LOG(APP_LOG_LEVEL_ERROR, "Inbox Received");

  MenuBrowser *browser = s_menu_browsers[s_browser_index];
  browser->loading_state = LOADING_STARTED;

  bool done = false;
  bool error = false;
  int num_entries = -1;
  char *alltitles = NULL;
  char *allsubtitles = NULL;
  char *allselectors = NULL;
  // char *msg_type = NULL;

  Tuple *t = dict_read_first(iterator);
  while (t != NULL)
  {
    switch(t->key)
    {
      case KEY_NUM_ENTRIES:
      {
        num_entries = (int)t->value->int32;
        break;
      }
      case KEY_TITLES:
      {
        alltitles = t->value->cstring;
        break;
      }
      case KEY_SELECTORS:
      {
        allselectors = t->value->cstring;
        break;
      }
      case KEY_SUBTITLES:
      {
        allsubtitles = t->value->cstring;
        break;
      }
      case KEY_DONE:
      {
        done = true;
        break;
      }
      case KEY_ERROR:
      {
        error = true;
        break;
      }
    }
    t = dict_read_next(iterator);
  }

  if (error)
  {
    Window *window = browser->menu_window;
    TextLayer *text_layer_error = get_text_layer_error(window);

    Layer *window_layer = window_get_root_layer(window);
    layer_add_child(window_layer, text_layer_get_layer(text_layer_error));
  }

  if (done)
  {
    browser->loading_state = LOADING_DONE;
  }

  if (!done)
  {
    if (alltitles)
    {
      if (browser->menu_titlecat == NULL)
      {
        browser->menu_titlecat = strdup(alltitles);
      }
      else
      {
        char *titlecat = calloc(strlen(browser->menu_titlecat) + strlen(alltitles) + 1, sizeof(char));
        strcat(titlecat, browser->menu_titlecat);
        strcat(titlecat, alltitles);
        free(browser->menu_titlecat);
        browser->menu_titlecat = titlecat;
      }
    }

    if (allsubtitles)
    {
      if (browser->menu_subtitlecat == NULL)
      {
        browser->menu_subtitlecat = strdup(allsubtitles);
      }
      else
      {
        char *subtitlecat = calloc(strlen(browser->menu_subtitlecat) + strlen(allsubtitles) + 1, sizeof(char));
        strcat(subtitlecat, browser->menu_subtitlecat);
        strcat(subtitlecat, allsubtitles);
        free(browser->menu_subtitlecat);
        browser->menu_subtitlecat = subtitlecat;
      }
    }

    if (allselectors)
    {
      if (browser->menu_selectorcat == NULL)
      {
        browser->menu_selectorcat = strdup(allselectors);
      }
      else
      {
        char *selectorcat = calloc(strlen(browser->menu_selectorcat) + strlen(allselectors) + 1, sizeof(char));
        strcat(selectorcat, browser->menu_selectorcat);
        strcat(selectorcat, allselectors);
        free(browser->menu_selectorcat);
        browser->menu_selectorcat = selectorcat;
      }
    }

    // printf("titlecat (%d): %s", browser->menu_titlecat ? strlen(browser->menu_titlecat) : 0, browser->menu_titlecat ? browser->menu_titlecat : "NULL");
    // printf("subtitlecat (%d): %s", browser->menu_subtitlecat ? strlen(browser->menu_subtitlecat) : 0, browser->menu_subtitlecat ? browser->menu_subtitlecat : "NULL");
    // printf("selectorcat (%d): %s", browser->menu_selectorcat ? strlen(browser->menu_selectorcat) : 0, browser->menu_selectorcat ? browser->menu_selectorcat : "NULL");
  }

  if (done)
  {
    Window *window = browser->menu_window;
    Layer *window_layer = window_get_root_layer(window);
    // printf("num entries: %d", num_entries);
    if (num_entries > 0)
    {
      browser->menu_num_entries = num_entries;
      browser->menu_titles = calloc(num_entries, sizeof(char *));
      browser->menu_selectors = calloc(num_entries, sizeof(char *));
      browser->menu_subtitles = calloc(num_entries, sizeof(char *));
      browser->menu_title_heights = calloc(num_entries, sizeof(int));
      browser->menu_subtitle_heights = calloc(num_entries, sizeof(int));

      char **titles = str_split(browser->menu_titlecat, '|');
      char **subtitles = browser->menu_subtitlecat ? str_split(browser->menu_subtitlecat, '|') : NULL;
      char **selectors = str_split(browser->menu_selectorcat, '|');
      for (int i=0; i<num_entries; i++)
      {
        char *title = titles[i];
        browser->menu_titles[i] = strdup(title);
        browser->menu_title_heights[i] = get_text_height(title, window_layer, FONT_KEY_GOTHIC_24_BOLD);
        free(title);

        char *selector = selectors[i];
        browser->menu_selectors[i] = strdup(selector);
        free(selector);

        browser->menu_subtitle_heights[i] = 0;
        if (subtitles)
        {
          char *subtitle = subtitles[i];
          browser->menu_subtitles[i] = strdup(subtitle);
          browser->menu_subtitle_heights[i] = get_text_height(subtitle, window_layer, FONT_KEY_GOTHIC_24);
          free(subtitle);
        }
      }
      free(titles);
      free(selectors);
      if (subtitles)
      {
        free(subtitles);
      }

      free(browser->menu_titlecat);
      browser->menu_titlecat = NULL;
      if (browser->menu_subtitlecat)
      {
        free(browser->menu_subtitlecat);
      }
      browser->menu_subtitlecat = NULL;
      free(browser->menu_selectorcat);
      browser->menu_selectorcat = NULL;

      if (!browser->menu_layer)
      {
        GRect bounds = layer_get_frame(window_layer);

        MenuLayer *menu_layer = menu_layer_create(bounds);
        browser->menu_layer = menu_layer;

        menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks){
          .get_num_sections = menu_get_num_sections_callback,
          .get_num_rows = menu_get_num_rows_callback,
          .get_header_height = menu_get_header_height_callback,
          .get_cell_height = menu_get_cell_height_callback,
          .draw_header = menu_draw_header_callback,
          .draw_row = menu_draw_row_callback,
          .select_click = menu_select_callback
        });

        menu_layer_set_click_config_onto_window(menu_layer, window);

        #ifdef PBL_COLOR
        menu_layer_colorize(menu_layer);
        #endif

        layer_add_child(window_layer, menu_layer_get_layer(menu_layer));
      }
      menu_layer_reload_data(browser->menu_layer);

      layer_remove_from_parent(text_layer_get_layer(get_text_layer_loading(NULL)));
    }
    else
    {
      setup_text_layer_noresults(window);
      layer_remove_child_layers(window_layer);
      layer_add_child(window_layer, text_layer_get_layer(s_text_layer_noresults));
    }
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context)
{
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context)
{
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context)
{
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
  browser->menu_subtitles = NULL;
  browser->menu_selectors = NULL;

  browser->menu_titlecat = NULL;
  browser->menu_subtitlecat = NULL;
  browser->menu_selectorcat = NULL;

  browser->menu_num_entries = 0;

  browser->loading_state = LOADING_NOT_STARTED;

  browser->msg = NULL;
  browser->route = NULL;
  browser->direction = NULL;
  browser->stopid = NULL;
  browser->stopname = NULL;
  browser->extra = NULL;
  browser->isfavorite = false;
}

static void free_browser_lists(MenuBrowser *browser)
{
  for (int i=0; i<browser->menu_num_entries; i++)
  {
    if (browser->menu_titles[i] != NULL)
    {
      free(browser->menu_titles[i]);
    }

    if (browser->menu_selectors[i] != NULL)
    {
      free(browser->menu_selectors[i]);
    }

    // subtitles are optional, so we need to check
    if (browser->menu_subtitles[i])
    {
      free(browser->menu_subtitles[i]);
    }
  }

  if (browser->menu_titles != NULL)
  {
    free(browser->menu_titles);
  }
  browser->menu_titles = NULL;
  if (browser->menu_subtitles != NULL)
  {
    free(browser->menu_subtitles);
  }
  browser->menu_subtitles = NULL;

  if (browser->menu_selectors != NULL)
  {
    free(browser->menu_selectors);
  }
  browser->menu_selectors = NULL;

  browser->menu_num_entries = 0;

  if (browser->menu_title_heights != NULL)
  {
    free(browser->menu_title_heights);
  }
  browser->menu_title_heights = NULL;
  if (browser->menu_subtitle_heights != NULL)
  {
    free(browser->menu_subtitle_heights);
  }
  browser->menu_subtitle_heights = NULL;
}

static void window_load(Window *window)
{
  #ifdef PBL_COLOR
    window_colorize(window);
  #endif
  TextLayer *text_layer_loading = get_text_layer_loading(window);

  Layer *window_layer = window_get_root_layer(window);
  layer_add_child(window_layer, text_layer_get_layer(text_layer_loading));
}

void reload_menu_browser_if_necessary()
{
  // the browser that'll soon be on the top of the stack
  // (s_browser_index should already be indexed to the backBrowser by now)
  MenuBrowser *backBrowser = s_menu_browsers[s_browser_index];

  if (backBrowser->loading_state != LOADING_DONE)
  {
    printf("back browser not finished loading!");
    menu_layer_reload_data(backBrowser->menu_layer);

    s_timer_browser_index = s_browser_index;
    s_timer = app_timer_register(
      250,
      send_menu_app_message_helper,
      NULL);
  }
  if (strcmp(backBrowser->msg, MSG_FAVORITES) == 0 || strcmp(backBrowser->msg, MSG_COMMUTE) == 0)
  {
    printf("returning to favorites / commute menu!");
    free_browser_lists(backBrowser);

    layer_add_child(window_get_root_layer(backBrowser->menu_window),
      text_layer_get_layer(get_text_layer_loading(NULL)));

    s_timer_browser_index = s_browser_index;
    s_timer = app_timer_register(
      250,
      send_menu_app_message_helper,
      NULL);
  }
}

static void window_unload(Window *window)
{
  // kill the timer so that we don't accidentally start sending
  // messages on a already-popped menu
  if (!s_timer_fired && s_timer != NULL)
  {
    app_timer_cancel(s_timer);
    s_timer_fired = false;
  }
  s_timer = NULL;

  MenuBrowser *browser = s_menu_browsers[s_browser_index];
  MenuLayer *menu_layer = browser->menu_layer;
  Window *menu_window = browser->menu_window;

  // Destroy the menu layer
  if (menu_layer != NULL)
  {
    menu_layer_destroy(menu_layer);
  }

  // Destroy the window
  if (menu_window != NULL)
  {
    window_destroy(menu_window);
  }

  free_browser_lists(browser);

  free(browser->msg);

  if (browser->route != NULL)
  {
    free(browser->route);
  }
  if (browser->direction != NULL)
  {
    free(browser->direction);
  }
  if (browser->stopid != NULL)
  {
    free(browser->stopid);
  }
  if (browser->stopname != NULL)
  {
    free(browser->stopname);
  }
  if (browser->extra != NULL)
  {
    free(browser->extra);
  }

  initialize_browser(browser); // set all to null
  free(browser);
  s_menu_browsers[s_browser_index] = NULL;

  if (s_browser_index > 0)
  {
    // index points to the browser that'll soon be on the top of the stack
    s_browser_index--;
    reload_menu_browser_if_necessary();
  }
  else
  {
    free(s_menu_browsers);
    s_menu_browsers = NULL;

    destroy_text_layer_loading();
    destroy_text_layer_error();
    text_layer_destroy(s_text_layer_noresults);
    s_text_layer_noresults = NULL;

    app_message_deregister_callbacks();
  }

  APP_LOG(APP_LOG_LEVEL_ERROR, "Unloaded");
}

void menu_browser_register_app_message_callbacks()
{
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
}

void push_menu(char *msg, char *route, char *direction, char *stopid, char *stopname, char *extra)
{
  if (s_menu_browsers == NULL)
  {
    int num_browsers = 4;
    s_menu_browsers = calloc(num_browsers, sizeof(MenuBrowser *));
    s_browser_index = 0;
  }
  else
  {
    s_browser_index++;
  }

  menu_browser_register_app_message_callbacks();

  MenuBrowser *browser = calloc(1, sizeof(MenuBrowser));
  initialize_browser(browser);
  s_menu_browsers[s_browser_index] = browser;

  // Create main Window element and assign to pointer
  Window *window = window_create();
  browser->menu_window = window;

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(window, true);

  // Add a key-value pair
  browser->msg = strdup(msg);
  if (route != NULL)
  {
    browser->route = strdup(route);
  }
  if (direction != NULL)
  {
    browser->direction = strdup(direction);
  }
  if (stopid != NULL)
  {
    browser->stopid = strdup(stopid);
  }
  if (stopname != NULL)
  {
    browser->stopname = strdup(stopname);
  }
  if (extra != NULL)
  {
    browser->extra = strdup(extra);
  }

  printf("launching app msg on timer @ browser index: %d", s_browser_index);
  s_timer_browser_index = s_browser_index;
  s_timer = app_timer_register(
    250,
    send_menu_app_message_helper,
    NULL);
}
