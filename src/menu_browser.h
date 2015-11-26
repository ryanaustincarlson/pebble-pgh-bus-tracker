#ifndef _MENU_BROWSER_H
#define _MENU_BROWSER_H

#include <pebble.h>

enum LOADING_STATE {LOADING_NOT_STARTED=0, LOADING_STARTED=1, LOADING_DONE=2};

typedef struct
{
  Window *menu_window;
  MenuLayer *menu_layer;
  char **menu_titles;
  char **menu_subtitles;
  char **menu_selectors; // what to send back to the phone
  int menu_num_entries;

  int *menu_title_heights;
  int *menu_subtitle_heights;

  enum LOADING_STATE loading_state;

  char *route;
  char *direction;
  char *stopid;
  char *stopname;
  char *msg;
  char *extra; // anything else we need to store
  bool isfavorite; // only applies to a (route,direction,stopid) tuple
} MenuBrowser;

void push_menu(char *msg,
    char *route,
    char *direction,
    char *stopid,
    char *stopname,
    char *extra);

#endif // _MENU_BROWSER_H
