#include "menu_browser.h"
#include "app_message_utils.h"

static Window *s_settings_window = NULL;
static MenuBrowser *s_menu_browser = NULL;
static TextLayer *s_text_favorites = NULL;
static TextLayer *s_text_morning_commute = NULL;
static TextLayer *s_text_evening_commute = NULL;

TextLayer *create_textlayer(GRect bounds, Layer *window_layer);

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
  Layer *window_layer = window_get_root_layer(window);
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

  text_layer_destroy(s_text_favorites);

}

void push_settings_actionbar(MenuBrowser *browser)
{
  s_menu_browser = browser;
  s_settings_window = window_create();

  window_set_window_handlers(s_settings_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });

  window_set_click_config_provider(s_settings_window, click_config_provider);

  window_stack_push(s_settings_window, true);
}
