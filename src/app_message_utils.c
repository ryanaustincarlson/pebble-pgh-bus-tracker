
#include "app_message_utils.h"

/*
 * MESSAGE SENDING
 */

void setup_app_message_dictionary(DictionaryIterator *iter, MenuBrowser *browser)
{
  if (browser->route != NULL)
  {
    dict_write_cstring(iter, 101, browser->route);
  }
  if (browser->direction != NULL)
  {
    dict_write_cstring(iter, 102, browser->direction);
  }
  if (browser->stopid != NULL)
  {
    dict_write_cstring(iter, 103, browser->stopid);
  }
  if (browser->stopname != NULL)
  {
    dict_write_cstring(iter, 104, browser->stopname);
  }
  if (browser->extra != NULL)
  {
    dict_write_cstring(iter, 105, browser->extra);
  }
}

void send_set_favorites_app_message(MenuBrowser *browser)
{
  // MenuBrowser *browser = s_menu_browsers[s_browser_index];

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  dict_write_cstring(iter, 100, "setfavorite");
  setup_app_message_dictionary(iter, browser);

  dict_write_int8(iter, 106, browser->isfavorite ? 1 : 0);

  app_message_outbox_send();
}

void send_menu_app_message(bool should_init, MenuBrowser *browser)
{
  // MenuBrowser *browser = s_menu_browsers[s_browser_index];

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  dict_write_cstring(iter, 100, browser->msg);
  setup_app_message_dictionary(iter, browser);

  dict_write_int8(iter, 106, should_init ? 1 : 0);

  // Send the message!
  app_message_outbox_send();
  // printf("sent %s message", browser->msg);
}
