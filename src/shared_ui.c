#include "shared_ui.h"
#include "app_colors.h"

static TextLayer *s_text_layer_loading = NULL;
static TextLayer *s_text_layer_error = NULL;
static TextLayer *s_text_layer_error_background = NULL;

TextLayer *get_text_layer_loading(Window *window)
{
  if (!s_text_layer_loading && window)
  {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_frame(window_layer);

    s_text_layer_loading = text_layer_create(bounds);
    text_layer_set_font(s_text_layer_loading, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_text_alignment(s_text_layer_loading, GTextAlignmentCenter);
    text_layer_set_text(s_text_layer_loading, "Loading...");
    #ifdef PBL_COLOR
      text_layer_set_text_color(s_text_layer_loading, get_color_normal());
      // text_layer_set_background_color(s_text_layer_loading, get_color_normal());
    #endif
  }

  return s_text_layer_loading;
}

void destroy_text_layer_loading()
{
  text_layer_destroy(s_text_layer_loading);
  s_text_layer_loading = NULL;
}

TextLayer *get_text_layer_error(Window *window)
{
  if (!s_text_layer_error && window)
  {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_frame(window_layer);

    s_text_layer_error = text_layer_create(bounds);
    text_layer_set_font(s_text_layer_error, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_text_alignment(s_text_layer_error, GTextAlignmentCenter);
    text_layer_set_text(s_text_layer_error, "Bus tracker\nis down!\nTry again\nsoon?\n*shrug*");
    #ifdef PBL_COLOR
      // text_layer_set_text_color(s_text_layer_error, get_color_error());
      text_layer_set_background_color(s_text_layer_error, get_color_error());
    #endif
  }

  return s_text_layer_error;
}

void destroy_text_layer_error()
{
  text_layer_destroy(s_text_layer_error);
  s_text_layer_error = NULL;
}
