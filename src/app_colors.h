/*
 * Colors!
 */

#ifdef PBL_COLOR
GColor get_color_error();
GColor get_color_normal();
void window_colorize(Window *window);
void menu_layer_colorize(MenuLayer *menu);
#endif