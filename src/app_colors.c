/*
 * Colors!
 */

#include <pebble.h>

#ifdef PBL_COLOR

GColor get_color_error()
{
  return GColorSunsetOrange;
}

GColor get_color_normal()
{
  return GColorCobaltBlue;
  // return GColorPictonBlue;
}

void window_colorize(Window *window)
{
  window_set_background_color(window, GColorWhite);
  // window_set_background_color(window, get_color_normal());
}

void menu_layer_colorize(MenuLayer *menu_layer)
{
  // menu, background color, foreground (text) color

  menu_layer_set_normal_colors(menu_layer, GColorWhite, GColorBlack);
  menu_layer_set_highlight_colors(menu_layer, get_color_normal(), GColorWhite);

  // menu_layer_set_normal_colors(menu_layer, get_color_normal(), GColorBlack);
  // menu_layer_set_highlight_colors(menu_layer, GColorBlack, get_color_normal());
}
#endif