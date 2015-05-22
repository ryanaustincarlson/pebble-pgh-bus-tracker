#include <pebble.h>
#include "menu_browser.h"

#include "app_constants.h"
#include "str_utils.h"

/*
 * STATIC VARS
 */

 enum LOADING_STATE {LOADING_NOT_STARTED=0, LOADING_STARTED=1, LOADING_DONE=2};

 typedef struct 
 {
  Window *menu_window;
  MenuLayer *menu_layer;
  char **menu_titles;
  char **menu_subtitles;
  char **menu_selectors; // what to send back to the phone
  int menu_num_entries;

  enum LOADING_STATE loading_state;

  char *route;
  char *direction;
  char *stopid;
  char *stopname;
  char *msg;
  bool isfavorite; // only applies to a (route,direction,stopid) tuple
} MenuBrowser;

// contains info for routes, direction, stops, predictions
static MenuBrowser **s_menu_browsers = NULL;
static int s_browser_index;

static TextLayer *s_text_layer_loading = NULL;
static TextLayer *s_text_layer_error = NULL;

// it'd be nice if these are all in a struct or something...
static AppTimer *s_timer = NULL; // timer that calls a fcn to send the first app msg
static bool s_timer_fired = false; // true if the timer has already fired
static int s_timer_browser_index = -1; // the browser index that we expect when the timer fires

/*
 * MESSAGE SENDING
 */

void setup_app_message_dictionary(DictionaryIterator *iter, MenuBrowser *browser)
{
  if (browser->route != NULL)
  {
    dict_write_cstring(iter, 1, browser->route);
  }
  if (browser->direction != NULL)
  {
    dict_write_cstring(iter, 2, browser->direction);
  }
  if (browser->stopid != NULL)
  {
    dict_write_cstring(iter, 3, browser->stopid);
  }
  if (browser->stopname != NULL)
  {
    dict_write_cstring(iter, 4, browser->stopname);
  }
}

void send_set_favorites_app_message()
{
  MenuBrowser *browser = s_menu_browsers[s_browser_index];

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  dict_write_cstring(iter, 0, "setfavorite");
  setup_app_message_dictionary(iter, browser);

  dict_write_int8(iter, 5, browser->isfavorite ? 1 : 0);

  app_message_outbox_send();
}

void send_menu_app_message(bool should_init)
{
  MenuBrowser *browser = s_menu_browsers[s_browser_index];

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  dict_write_cstring(iter, 0, browser->msg);
  setup_app_message_dictionary(iter, browser);

  dict_write_int8(iter, 5, should_init ? 1 : 0);

  // Send the message!
  app_message_outbox_send();
  // printf("sent %s message", msg);
}

void send_menu_app_message_helper(void *should_init_ptr)
{
  s_timer_fired = true;
  if (s_timer_browser_index == s_browser_index)
  {
    printf("timer browser idx: %d, browser_index: %d", s_timer_browser_index, s_browser_index);
    bool should_init = (bool)should_init_ptr;
    printf("sending menu app msg");
    send_menu_app_message(should_init);
  }
}

/*
 * MENU
 */

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  MenuBrowser *browser = s_menu_browsers[s_browser_index];
  if (strcmp(browser->msg, MSG_PREDICTIONS) == 0)
    return 2;
  return 1;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  MenuBrowser *browser = s_menu_browsers[s_browser_index];
  int num_entries = browser->menu_num_entries;

  if (strcmp(browser->msg, MSG_PREDICTIONS) == 0)
  {
    if (section_index == 0)
    {
      return 1;
    }
    else if (section_index == 1)
    {
      return num_entries;
    }
  }
  else if (section_index == 0)
  {
    return num_entries;
  }
  return 0;
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  MenuBrowser *browser = s_menu_browsers[s_browser_index];
  char *msg = browser->msg;
  
  char *header = NULL;
  if (strcmp(msg, MSG_ROUTES) == 0)
    header = "Routes";
  else if (strcmp(msg, MSG_DIRECTIONS) == 0)
    header = "Directions";
  else if (strcmp(msg, MSG_STOPS) == 0)
    header = "Stops";
  else if (strcmp(msg, MSG_PREDICTIONS) == 0)
    header = "Predictions";

  bool on_prediction_screen = strcmp(browser->msg, MSG_PREDICTIONS) == 0;
  if ((on_prediction_screen && section_index == 1) || (!on_prediction_screen && section_index == 0))
  {
    menu_cell_basic_header_draw(ctx, cell_layer, header);
  }
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  MenuBrowser *browser = s_menu_browsers[s_browser_index];
  char *title = browser->menu_titles[cell_index->row];
  char *subtitle = browser->menu_subtitles[cell_index->row];

  char *favorite_msg = NULL;
  if (!browser->isfavorite)
  {
    favorite_msg = "Mark as Favorite";
  } 
  else
  {
    favorite_msg = "Clear Favorite";
  }

  bool on_prediction_screen = strcmp(browser->msg, MSG_PREDICTIONS) == 0;
  int content_section_index = on_prediction_screen ? 1 : 0;
  if (on_prediction_screen)
  {
    if (cell_index->section ==  0)
    {
      menu_cell_basic_draw(ctx, cell_layer, favorite_msg, NULL, NULL);
    }
  }

  if (cell_index->section == content_section_index)
  {
    menu_cell_basic_draw(ctx, cell_layer, title, subtitle, NULL);
  }
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  MenuBrowser *browser = s_menu_browsers[s_browser_index];

  bool on_prediction_screen = strcmp(browser->msg, MSG_PREDICTIONS) == 0;
  int section_index = cell_index->section;
  if ((on_prediction_screen && section_index == 1) || (!on_prediction_screen && section_index == 0))
  {
    char *msg = browser->msg;
    char *route = browser->route;
    char *direction = browser->direction;
    char *stopid = browser->stopid;
    char *stopname = browser->stopname;

    char *selector = browser->menu_selectors[cell_index->row];
    char *new_msg = NULL;

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

    if (new_msg)
    {
      printf("pushing menu with route: %s, direction: %s, stopid: %s, stopname: %s",
        route, direction, stopid, stopname);

      s_browser_index++;
      push_menu(new_msg, route, direction, stopid, stopname);
    }
  }
  // otherwise check that we ARE on prediction screen and we're selecting the favorites button
  else if (on_prediction_screen && section_index == 0)
  {
    // first we swap isfavorite bit
    browser->isfavorite = !browser->isfavorite;
    send_set_favorites_app_message();

    menu_layer_reload_data(browser->menu_layer);
  }
}

/*
 * APP MESSAGES
 */

static void inbox_received_callback(DictionaryIterator *iterator, void *context) 
{
  // APP_LOG(APP_LOG_LEVEL_ERROR, "Inbox Received");

  MenuBrowser *browser = s_menu_browsers[s_browser_index];
  browser->loading_state = LOADING_STARTED;

  bool done = false;
  int num_entries = -1;
  int item_index = -1;
  char *title = NULL;
  char *subtitle = NULL;
  char *selector = NULL;
  char *msg_type = NULL;

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
      case KEY_SELECTORS:
      {
        selector = t->value->cstring;
        break;
      }
      case KEY_SUBTITLES:
      {
        subtitle = t->value->cstring;
        break;
      }
      case KEY_MSG_TYPE:
      {
        msg_type = t->value->cstring;
        break;
      }
      case KEY_IS_FAVORITE:
      {
        browser->isfavorite = (int)t->value->int32 == 1;
        printf("setting is favorite: %s", browser->isfavorite ? "yes!" : "no!");
        break;
      }
    }

    if (strcmp("done", t->value->cstring) == 0)
    {
      // APP_LOG(APP_LOG_LEVEL_ERROR, "Done finding messages!");
      done = true;
      browser->loading_state = LOADING_DONE;
    }

    t = dict_read_next(iterator);
  }

  // we want to make sure we're creating space for entries
  // in the proper browser
  //
  // this is mostly to protect against wonkiness if a user hits
  // the back button before the first entry has loaded
  if (num_entries > 0 && strcmp(msg_type, browser->msg) == 0)
  {
    browser->menu_titles = calloc(num_entries, sizeof(char *));
    browser->menu_selectors = calloc(num_entries, sizeof(char *));
    browser->menu_subtitles = calloc(num_entries, sizeof(char *));
  }

  Window *window = browser->menu_window;
  Layer *window_layer = window_get_root_layer(window);

  if (browser->menu_num_entries > 0)
  {
    if (!browser->menu_layer)
    {
      GRect bounds = layer_get_frame(window_layer);

      MenuLayer *menu_layer = menu_layer_create(bounds);
      s_menu_browsers[s_browser_index]->menu_layer = menu_layer;

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
    }
    menu_layer_reload_data(browser->menu_layer);
  }

  if (done)
  {
    if (browser->menu_num_entries == 0)
    {
      layer_add_child(window_layer, text_layer_get_layer(s_text_layer_error));
    }
  }
  else // not done - keep going
  {
    MenuBrowser *onscreen_browser = browser;

    // if there's an errant message from the previous screen,
    // we need to catch it and place it in the CORRECT browser
    if (strcmp(msg_type, browser->msg) != 0)
    {
      printf("LOADING was interrupted w/ msg_type = %s", msg_type);
      browser = NULL;
      for (int msgIdx=0; msgIdx<s_browser_index; msgIdx++)
      {
        MenuBrowser *tmp_browser = s_menu_browsers[msgIdx];
        if (strcmp(msg_type, tmp_browser->msg) == 0)
        {
          printf("FOUND replacement at index = %d", msgIdx);
          browser = tmp_browser;
          break;
        }
      }
    }

    if (item_index != -1 && browser != NULL)
    {
      if (item_index+1 > browser->menu_num_entries)
      {
        browser->menu_num_entries = item_index+1;
      }

      browser->menu_titles[item_index] = strdup(title);
      browser->menu_selectors[item_index] = strdup(selector);

      if (subtitle != NULL)
      {
        browser->menu_subtitles[item_index] = strdup(subtitle);
      }

      printf("idx: %d, title: %s, subt: %s, sel: %s", 
        item_index, 
        browser->menu_titles[item_index], 
        browser->menu_subtitles[item_index], 
        browser->menu_selectors[item_index]);
    }

    if (onscreen_browser == browser)
    {
      send_menu_app_message(false);
    }
  }
}

 static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
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
  browser->menu_subtitles = NULL;
  browser->menu_selectors = NULL;
  browser->menu_num_entries = 0;

  browser->loading_state = LOADING_NOT_STARTED;

  browser->msg = NULL;
  browser->route = NULL;
  browser->direction = NULL;
  browser->stopid = NULL;
  browser->stopname = NULL;
  browser->isfavorite = false;
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  if (!s_text_layer_loading)
  {
    // TODO: make this prettier!
    s_text_layer_loading = text_layer_create(bounds);
    text_layer_set_text(s_text_layer_loading, "Loading...");
  }
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer_loading));

  if (!s_text_layer_error)
  {
    // TODO: make this prettier!
    s_text_layer_error = text_layer_create(bounds);
    text_layer_set_text(s_text_layer_error, "Error! No results found.");
  } 
}

static void window_unload(Window *window) {
  printf("window unloading...");
  // kill the timer so that we don't accidentally start sending
  // messages on a already-popped menu
  if (!s_timer_fired && s_timer != NULL)
  {
    app_timer_cancel(s_timer);
    s_timer_fired = false;
  }
  s_timer = NULL;

  // int browser_index = s_browser_index--;

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

  printf("num entries: %d", browser->menu_num_entries);
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
  free(browser->menu_titles);
  free(browser->menu_subtitles);
  free(browser->menu_selectors);

  browser->menu_num_entries = 0;

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

  initialize_browser(browser); // set all to null
  free(browser);
  s_menu_browsers[s_browser_index] = NULL;
  
  if (s_browser_index > 0)
  {
    s_browser_index--;

    // the browser that'll soon be on the top of the stack
    MenuBrowser *backBrowser = s_menu_browsers[s_browser_index];
    if (backBrowser->loading_state != LOADING_DONE)
    {
      printf("back browser not finished loading!");
      menu_layer_reload_data(backBrowser->menu_layer);
      bool should_init = backBrowser->loading_state == LOADING_NOT_STARTED;
      send_menu_app_message(should_init);
    }
  }
  else
  {
    free(s_menu_browsers);
    s_menu_browsers = NULL;

    text_layer_destroy(s_text_layer_loading);
    text_layer_destroy(s_text_layer_error);
    s_text_layer_loading = NULL;
    s_text_layer_error = NULL;

    app_message_deregister_callbacks();
  }

  APP_LOG(APP_LOG_LEVEL_ERROR, "Unloaded");
}

void push_menu(char *msg, char *route, char *direction, char *stopid, char *stopname)
{ 
  if (s_menu_browsers == NULL)
  {
    printf("first push menu !");
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

  // printf("created browser: %p", browser);
  // for (int i=0; i<4; i++)
  // {
  //   printf("menu_browsers[%d] = %p", i, s_menu_browsers[i]);
  // }

  // Create main Window element and assign to pointer
  Window *window = window_create();
  browser->menu_window = window;

  // printf("created menu_window: %p", window);
  
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
  // send_menu_app_message(true);

  s_timer_browser_index = s_browser_index;
  s_timer = app_timer_register(
    500, 
    send_menu_app_message_helper, 
    (void *)true);
}
