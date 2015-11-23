#include "menu_browser.h"

static Window *settings_window = NULL;

void window_load(Window *window)
{
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  TextLayer *text_layer = text_layer_create(bounds);
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  text_layer_set_text(text_layer, "Hold on!");

  layer_add_child(window_layer, text_layer_get_layer(text_layer));
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
