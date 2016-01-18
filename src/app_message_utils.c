
#include "app_message_utils.h"

/*
 * MESSAGE SENDING
 */

void setup_app_message_dictionary(DictionaryIterator *iter, MenuBrowser *browser)
{
  // send along the max inbox size
  // (later: figure out the CURRENT inbox size rather than the )
  dict_write_uint32(iter, 150, app_message_inbox_size_maximum());

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

// this is a helper for the "set favorites" and "set commute" functions
void send_set_property_app_message(MenuBrowser *browser, char *keyword, bool shouldAdd)
{
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  dict_write_cstring(iter, 100, keyword);
  setup_app_message_dictionary(iter, browser);

  dict_write_int8(iter, 106, shouldAdd ? 1 : 0);

  app_message_outbox_send();
}

void send_set_favorites_app_message(MenuBrowser *browser)
{
  send_set_property_app_message(browser, "setfavorite", browser->isfavorite ? 1 : 0);
}

void send_set_morning_commute_app_message(MenuBrowser *browser)
{
  send_set_property_app_message(browser, "setmorningcommute", browser->ismorningcommute ? 1 : 0);
}

void send_set_evening_commute_app_message(MenuBrowser *browser)
{
  send_set_property_app_message(browser, "seteveningcommute", browser->iseveningcommute ? 1 : 0);
}

void send_get_saved_data_app_message(MenuBrowser *browser)
{
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  dict_write_cstring(iter, 100, "getsaveddata");
  setup_app_message_dictionary(iter, browser);

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
