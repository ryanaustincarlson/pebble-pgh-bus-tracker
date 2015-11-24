#include "menu_browser.h"

static Window *settings_window = NULL;

TextLayer *create_textlayer(GRect bounds, Layer *window_layer);

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
  TextLayer *top_text_layer = create_textlayer(top_bounds, window_layer);
  text_layer_set_text(top_text_layer, "Add to AM Commute");

  // center text
  GPoint center_origin;
  center_origin.x = left_x;
  center_origin.y = size.h/3;
  GRect center_bounds;
  center_bounds.origin = center_origin;
  center_bounds.size = segment_size;
  TextLayer *center_text_layer = create_textlayer(center_bounds, window_layer);
  text_layer_set_text(center_text_layer, "Add to Favorites");

  // bottom text
  GPoint bottom_origin;
  bottom_origin.x = left_x;
  bottom_origin.y = 2 * size.h/3;
  GRect bottom_bounds;
  bottom_bounds.origin = bottom_origin;
  bottom_bounds.size = segment_size;
  TextLayer *bottom_text_layer = create_textlayer(bottom_bounds, window_layer);
  text_layer_set_text(bottom_text_layer, "Add to PM Commute");
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
  window_destroy(settings_window);
}

void push_settings_actionbar(MenuBrowser *browser)
{
  printf("hi!");
  settings_window = window_create();

  window_set_window_handlers(settings_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });

  window_stack_push(settings_window, true);
}
