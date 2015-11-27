
#ifndef _APP_MESSAGE_UTILS_H
#define _APP_MESSAGE_UTILS_H

#include <pebble.h>

#include "menu_browser.h"

void setup_app_message_dictionary(DictionaryIterator *iter, MenuBrowser *browser);
void send_set_favorites_app_message(MenuBrowser *browser);
void send_set_morning_commute_app_message(MenuBrowser *browser);
void send_set_evening_commute_app_message(MenuBrowser *browser);
void send_menu_app_message(bool should_init, MenuBrowser *browser);

#endif // _APP_MESSAGE_UTILS_H
