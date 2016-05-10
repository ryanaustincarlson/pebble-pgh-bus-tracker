#ifndef _SHARED_UI_H
#define _SHARED_UI_H

#include <pebble.h>

TextLayer *get_text_layer_loading(Window *window);
void destroy_text_layer_loading();

TextLayer *get_text_layer_error(Window *window);
void destroy_text_layer_error();

#endif // _SHARED_UI_H
