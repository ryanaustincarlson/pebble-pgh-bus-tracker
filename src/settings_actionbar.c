#include "menu_browser.h"
#include "app_message_utils.h"
#include "str_utils.h"
#include "app_constants.h"
#include "shared_ui.h"
#include "app_colors.h"

static Window *s_settings_window = NULL;
static MenuBrowser *s_menu_browser = NULL;
static TextLayer *s_text_favorites = NULL;
static TextLayer *s_text_morning_commute = NULL;
static TextLayer *s_text_evening_commute = NULL;

static TextLayer *create_textlayer(GRect bounds, Layer *window_layer);
static void window_load(Window *window);
static void setup_commute_text();

/*
 * App Messages
 */

static void inbox_received_callback(DictionaryIterator *iterator, void *context)
{
  APP_LOG(APP_LOG_LEVEL_ERROR, "Inbox Received");

  bool is_favorite = false;
  bool is_am_commute = false;
  bool is_pm_commute = false;

  Tuple *t = dict_read_first(iterator);
  while (t != NULL)
  {
    switch(t->key)
    {
      case KEY_IS_FAVORITE:
      {
        is_favorite = (int)t->value->int32 == 1;
        break;
      }
      case KEY_IS_MORNING_COMMUTE:
      {
        is_am_commute = (int)t->value->int32 == 1;
        break;
      }
      case KEY_IS_EVENING_COMMUTE:
      {
        is_pm_commute = (int)t->value->int32 == 1;
        break;
      }
    }

    t = dict_read_next(iterator);
  }

  printf("is fav? %s, is AM? %s, is PM? %s",
          is_favorite ? "yes" : "no",
          is_am_commute ? "yes" : "no",
          is_pm_commute ? "yes" : "no");

  s_menu_browser->isfavorite = is_favorite;
  s_menu_browser->ismorningcommute = is_am_commute;
  s_menu_browser->iseveningcommute = is_pm_commute;

  // window_load(s_settings_window);
  setup_commute_text();
  layer_remove_from_parent(text_layer_get_layer(get_text_layer_loading(NULL)));
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
 * setup text
 */
static void setup_favorites_text()
{
  char *text;
  if (s_menu_browser->isfavorite)
  {
    text = "Remove From Favorites";
  }
  else
  {
    text = "Add To Favorites";
  }
  text_layer_set_text(s_text_favorites, text);
}

static void setup_morning_commute_text()
{
  char *text;
  if (s_menu_browser->ismorningcommute)
  {
    text = "Remove From AM Commute";
  }
  else
  {
    text = "Add To AM Commute";
  }
  text_layer_set_text(s_text_morning_commute, text);
}

static void setup_evening_commute_text()
{
  char *text;
  if (s_menu_browser->iseveningcommute)
  {
    text = "Remove From PM Commute";
  }
  else
  {
    text = "Add To PM Commute";
  }
  text_layer_set_text(s_text_evening_commute, text);
}

static void setup_commute_text()
{
  Layer *window_layer = window_get_root_layer(s_settings_window);
  GRect bounds = layer_get_frame(window_layer);

  GPoint origin = bounds.origin;
  GSize size = bounds.size;

  GSize segment_size;
  segment_size.w = size.w * 0.75;
  segment_size.h = size.h/3;

  int left_x = 20;

  // top text
  GPoint top_origin;
  top_origin.x = left_x;
  top_origin.y = 5;
  GRect top_bounds;
  top_bounds.origin = top_origin;
  top_bounds.size = segment_size;
  s_text_morning_commute = create_textlayer(top_bounds, window_layer);
  setup_morning_commute_text();

  // center text
  GPoint center_origin;
  center_origin.x = left_x;
  center_origin.y = size.h/3;
  GRect center_bounds;
  center_bounds.origin = center_origin;
  center_bounds.size = segment_size;
  s_text_favorites = create_textlayer(center_bounds, window_layer);
  setup_favorites_text();

  // bottom text
  GPoint bottom_origin;
  bottom_origin.x = left_x;
  bottom_origin.y = 2 * size.h/3;
  GRect bottom_bounds;
  bottom_bounds.origin = bottom_origin;
  bottom_bounds.size = segment_size;
  s_text_evening_commute = create_textlayer(bottom_bounds, window_layer);
  setup_evening_commute_text();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  printf("up clicked");
  s_menu_browser->ismorningcommute = !s_menu_browser->ismorningcommute;
  send_set_morning_commute_app_message(s_menu_browser);
  setup_morning_commute_text();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  printf("select clicked");
  s_menu_browser->isfavorite = !s_menu_browser->isfavorite;
  send_set_favorites_app_message(s_menu_browser);
  setup_favorites_text();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  printf("down clicked");
  s_menu_browser->iseveningcommute = !s_menu_browser->iseveningcommute;
  send_set_evening_commute_app_message(s_menu_browser);
  setup_evening_commute_text();
}

static void click_config_provider(void *context) {
  // Register the ClickHandlers
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

void window_load(Window *window)
{
  #ifdef PBL_COLOR
    window_colorize(window);
  #endif
  TextLayer *text_layer_loading = get_text_layer_loading(window);

  Layer *window_layer = window_get_root_layer(window);
  layer_add_child(window_layer, text_layer_get_layer(text_layer_loading));
}

TextLayer *create_textlayer(GRect bounds, Layer *window_layer)
{
  TextLayer *text_layer = text_layer_create(bounds);
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(text_layer, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
  return text_layer;
}

void window_unload(Window *window)
{
  window_destroy(s_settings_window);

  if (s_menu_browser->route != NULL)
  {
    free(s_menu_browser->route);
  }
  if (s_menu_browser->direction != NULL)
  {
    free(s_menu_browser->direction);
  }
  if (s_menu_browser->stopid != NULL)
  {
    free(s_menu_browser->stopid);
  }
  if (s_menu_browser->stopname != NULL)
  {
    free(s_menu_browser->stopname);
  }
  if (s_menu_browser->extra != NULL)
  {
    free(s_menu_browser->extra);
  }
  free(s_menu_browser);
  s_menu_browser = NULL;

  text_layer_destroy(s_text_favorites);
  s_text_favorites = NULL;

  text_layer_destroy(s_text_morning_commute);
  s_text_morning_commute = NULL;

  text_layer_destroy(s_text_evening_commute);
  s_text_evening_commute = NULL;

  app_message_deregister_callbacks();
  menu_browser_register_app_message_callbacks();
  reload_menu_browser_if_necessary();
}

void push_settings_actionbar(char *route, char *direction, char *stopid, char *stopname)
{
  printf("pushing actionbar w/ route: %s, dir: %s, stopid: %s, stopname: %s",
    route ? route : "null", direction ? direction : "null",
    stopid ? stopid : "null", stopname ? stopname : "null");
  s_menu_browser = calloc(1, sizeof(MenuBrowser));
  s_menu_browser->route = strdup(route);
  s_menu_browser->direction = strdup(direction);
  s_menu_browser->stopid = strdup(stopid);
  s_menu_browser->stopname = strdup(stopname);

  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  send_get_saved_data_app_message(s_menu_browser);

  s_settings_window = window_create();

  window_set_window_handlers(s_settings_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });

  window_set_click_config_provider(s_settings_window, click_config_provider);

  window_stack_push(s_settings_window, true);
}
