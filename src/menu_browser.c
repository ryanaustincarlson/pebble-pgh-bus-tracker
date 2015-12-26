#include "menu_browser.h"
#include "str_utils.h"

#include "app_constants.h"
#include "app_colors.h"
#include "app_message_utils.h"

#include "settings_actionbar.h"
#include "shared_ui.h"

/*
 * CONSTANTS / STATIC VARS
 */

#define HORIZ_SCROLL_WAIT_TIME 750 // ms
#define HORIZ_SCROLL_ITEM_TIME 150 // ms
#define HORIZ_SCROLL_VISIBLE_CHARS 15

#define MIN_ROW_HEIGHT 44
#define MAX_ROW_HEIGHT (int)MIN_ROW_HEIGHT * 2.5

static const bool ENABLE_HORIZONTAL_SCROLLING = false;

static void selected_index_monitor(void *data);

// contains info for routes, direction, stops, predictions
static MenuBrowser **s_menu_browsers = NULL;
static int s_browser_index;

static TextLayer *s_text_layer_noresults = NULL;

// it'd be nice if these are all in a struct or something...
static AppTimer *s_timer = NULL; // timer that calls a fcn to send the first app msg
static bool s_timer_fired = false; // true if the timer has already fired
static int s_timer_browser_index = -1; // the browser index that we expect when the timer fires

static AppTimer *s_horiz_scroll_timer = NULL;
static bool s_horiz_scroll_timer_active = false; // true if timer has been scheduled
static unsigned long s_horiz_scroll_timer_timestamp = 0; // in milliseconds
static int s_horiz_scroll_offset = 0;
static bool s_horiz_scroll_scrolling_still_required = true;
static int s_horiz_scroll_menu_index = -1;
// static bool s_horiz_scroll_menu_reloading_to_scroll = false;

static int s_menu_selection_index = -1; // our currently selected item (row only for now)

/*
 * Utilty Functions
 */
unsigned long get_timestamp()
{
  time_t seconds;
  uint16_t millis;
  time_ms(&seconds, &millis);
  unsigned long timestamp = seconds * 1000 + millis; // report milliseconds
  return timestamp;
}

int get_text_height(char *text, const Layer *layer, const char *font_key)
{
  GRect bounds = layer_get_frame(layer);
  int cell_width = bounds.size.w - 10;
  GSize size = graphics_text_layout_get_content_size(
    text,
    fonts_get_system_font(font_key),
    GRect(5, 0, cell_width, MAX_ROW_HEIGHT),
    GTextOverflowModeWordWrap,
    GTextAlignmentLeft);
  return size.h;
}

/*
 * Text Layers
 */

void setup_text_layer_noresults(Window *window)
{
  if (!s_text_layer_noresults)
  {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_frame(window_layer);

    s_text_layer_noresults = text_layer_create(bounds);
    text_layer_set_font(s_text_layer_noresults, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_text_alignment(s_text_layer_noresults, GTextAlignmentCenter);
    #ifdef PBL_COLOR
      // text_layer_set_text_color(s_text_layer_noresults, get_color_error());
      text_layer_set_background_color(s_text_layer_noresults, get_color_error());
    #endif
  }

  MenuBrowser *browser = s_menu_browsers[s_browser_index];
  char *keyword = NULL;
  if (strcmp(browser->msg, MSG_ROUTES) == 0)
  {
    keyword = "routes";
  }
  else if (strcmp(browser->msg, MSG_DIRECTIONS) == 0)
  {
    keyword = "directions";
  }
  else if (strcmp(browser->msg, MSG_STOPS) == 0)
  {
    keyword = "stops";
  }
  else if (strcmp(browser->msg, MSG_PREDICTIONS) == 0)
  {
    keyword = "predictions";
  }
  else if (strcmp(browser->msg, MSG_FAVORITES) == 0)
  {
    keyword = "favorites";
  }
  else if (strcmp(browser->msg, MSG_NEARBY_STOPS) == 0)
  {
    keyword = "nearby stops";
  }
  else if (strcmp(browser->msg, MSG_NEARBY_ROUTES) == 0)
  {
    keyword = "nearby routes";
  }
  else if (strcmp(browser->msg, MSG_COMMUTE) == 0)
  {
    keyword = "commute routes";
  }

  char *start = "No ";
  char *end = "\nto display.";
  char *message = calloc(strlen(start) + strlen(keyword) + strlen(end) + 1, sizeof(char));
  strcat(message, start);
  strcat(message, keyword);
  strcat(message, end);
  text_layer_set_text(s_text_layer_noresults, message);
  free(message);
}

void send_menu_app_message_helper(void *should_init_ptr)
{
  s_timer_fired = true;
  printf("in menu app msg helper");
  printf("browser index...");
  printf("%d", s_browser_index);
  if (s_timer_browser_index == s_browser_index)
  {
    printf("timer browser idx: %d, browser_index: %d", s_timer_browser_index, s_browser_index);
    bool should_init = (bool)should_init_ptr;
    printf("sending menu app msg");
    send_menu_app_message(should_init, s_menu_browsers[s_browser_index]);
  }
  printf("ahhhh");
}

/*
 * MENU
 */

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data)
{
  return 1;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data)
{
  MenuBrowser *browser = s_menu_browsers[s_browser_index];
  int num_entries = browser->menu_num_entries;

  return num_entries;
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data)
{
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static int16_t menu_get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data)
{
  MenuBrowser *browser = s_menu_browsers[s_browser_index];

  int title_height = browser->menu_title_heights[cell_index->row];
  int subtitle_height = browser->menu_subtitle_heights[cell_index->row];
  return  title_height + (subtitle_height + 1) + 8;
}

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data)
{
  MenuBrowser *browser = s_menu_browsers[s_browser_index];
  char *msg = browser->msg;

  char *header = NULL;
  if (strcmp(msg, MSG_ROUTES) == 0)
  {
    header = "Routes";
  }
  else if (strcmp(msg, MSG_DIRECTIONS) == 0)
  {
    header = "Directions";
  }
  else if (strcmp(msg, MSG_STOPS) == 0)
  {
    header = "Stops";
  }
  else if (strcmp(msg, MSG_PREDICTIONS) == 0)
  {
    header = "Predictions";
  }
  else if (strcmp(msg, MSG_FAVORITES) == 0)
  {
    header = "Favorites";
  }
  else if (strcmp(msg, MSG_NEARBY_STOPS) == 0)
  {
    header = "Nearby Stops";
  }
  else if (strcmp(msg, MSG_NEARBY_ROUTES) == 0)
  {
    header = "Nearby Routes";
  }
  else if (strcmp(msg, MSG_COMMUTE) == 0)
  {
    header = "Commute";
  }

  menu_cell_basic_header_draw(ctx, cell_layer, header);
}

static char *get_menu_item_text(MenuLayer *menu_layer, int index, char *text)
{
  char *modified = text;

  MenuIndex menu_index = menu_layer_get_selected_index(menu_layer);
  if (modified && menu_index.row == index)
  {
    int len = strlen(modified);

    // if (len - HORIZ_SCROLL_VISIBLE_CHARS - s_horiz_scroll_offset > 0)
    bool is_text_too_long = len - HORIZ_SCROLL_VISIBLE_CHARS > 0;

    int offset_strlen = len - s_horiz_scroll_offset;
    bool keep_scrolling = offset_strlen >= 0;
    if (is_text_too_long && keep_scrolling)
    {
      if (offset_strlen == 0)
      {
        modified = "";
        s_horiz_scroll_scrolling_still_required = true;
      }
      else if (offset_strlen == -1)
      {
        modified = text;
      }
      else
      {
        modified += s_horiz_scroll_offset;
        s_horiz_scroll_scrolling_still_required = true;
      }
    }
  }
  return modified;
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data)
{
  MenuBrowser *browser = s_menu_browsers[s_browser_index];

  char *title = browser->menu_titles[cell_index->row];
  char *subtitle = browser->menu_subtitles[cell_index->row];

  char *newtitle = get_menu_item_text(browser->menu_layer, cell_index->row, title);

  // menu_cell_basic_draw(ctx, cell_layer, newtitle, subtitle, NULL);

  GRect bounds = layer_get_frame(cell_layer);
  // printf("origin: (%d, %d), size: (%d, %d)", bounds.origin.x, bounds.origin.y, bounds.size.w, bounds.size.h);

  int title_height = browser->menu_title_heights[cell_index->row];
  int subtitle_height = browser->menu_subtitle_heights[cell_index->row];
  int cell_width = bounds.size.w - 10;

  graphics_draw_text(ctx, newtitle,
    fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(5, 0, cell_width, title_height),
    GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
    graphics_draw_text(ctx, subtitle,
      fonts_get_system_font(FONT_KEY_GOTHIC_24), GRect(5, title_height + 1, cell_width, subtitle_height),
      GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
}

static void horiz_scroll_callback(void *data)
{
  if (!ENABLE_HORIZONTAL_SCROLLING)
  {
    return;
  }

  // printf("in horiz scroll callback");
  s_horiz_scroll_timer = NULL;
  s_horiz_scroll_offset++;

  MenuBrowser *browser = s_menu_browsers[s_browser_index];
  MenuLayer *menu_layer = NULL;
  MenuIndex menu_index;

  bool should_scroll = false;
  if (browser != NULL && browser->menu_layer != NULL)
  {
    // menu is initialized so let's check if we should scroll
    menu_layer = browser->menu_layer;
    menu_index = menu_layer_get_selected_index(menu_layer);

    should_scroll = s_horiz_scroll_scrolling_still_required && s_menu_selection_index == menu_index.row;
  }

  if (should_scroll)
  {
    // printf("scroll_callback ~> legit browser && menu layer!");

    // TODO: check that the callback fired at the same index as current browser!

    char *title = browser->menu_titles[menu_index.row];

    printf("horiz scroll..: %s", title+s_horiz_scroll_offset);

    // because in row==0, menu_selection_changed doesn't get called
    // if (menu_index.row != 0)
    // {
    //   s_horiz_scroll_menu_reloading_to_scroll = true;
    // }
    s_horiz_scroll_scrolling_still_required = false;
    // menu_layer_reload_data(menu_layer);
    layer_mark_dirty(menu_layer_get_layer(menu_layer));
    s_horiz_scroll_timer = app_timer_register(HORIZ_SCROLL_ITEM_TIME, horiz_scroll_callback, NULL); // FIXME
    s_horiz_scroll_timer_active = true;
  }
  else
  {
    s_horiz_scroll_offset = 0;
    // s_horiz_scroll_timer_active = false;
    s_horiz_scroll_timer = app_timer_register(HORIZ_SCROLL_WAIT_TIME, selected_index_monitor, NULL); // FIXME
  }
}

static void selected_index_monitor(void *data)
{
  // printf("monitor called"); // FIXME: useful for debugging, but need to take out at some point...

  unsigned long timestamp = get_timestamp();
  int difference = timestamp - s_horiz_scroll_timer_timestamp;
  // printf("difference: %d, old: %lu, new: %lu", difference, s_horiz_scroll_timer_timestamp, timestamp);

  MenuBrowser *browser = s_menu_browsers[s_browser_index];
  MenuIndex menu_index;

  bool should_scroll = false;
  if (browser != NULL && browser->menu_layer != NULL)
  {
    // printf("legit browser & menu layer!");
    menu_index = menu_layer_get_selected_index(browser->menu_layer);
    s_menu_selection_index = menu_index.row;

    if (s_horiz_scroll_timer_timestamp == 0)
    {
      s_horiz_scroll_timer_timestamp = timestamp;
    }
    else if (s_menu_selection_index != menu_index.row)
    {
      s_horiz_scroll_timer_timestamp = timestamp;
    }

    should_scroll = s_menu_selection_index == menu_index.row && s_horiz_scroll_menu_index != menu_index.row && difference >= HORIZ_SCROLL_WAIT_TIME;
  }

  if (should_scroll)
  {
    // start scrolling
    s_horiz_scroll_scrolling_still_required = true;
    s_horiz_scroll_timer_timestamp = timestamp;
    s_horiz_scroll_offset = 0;
    s_horiz_scroll_menu_index = menu_index.row;

    horiz_scroll_callback(NULL);
  }
  else
  {
    // otherwise, keep checking the selected index
    s_horiz_scroll_timer = app_timer_register(HORIZ_SCROLL_WAIT_TIME, selected_index_monitor, NULL); // FIXME
  }
}

static void menu_selection_changed_callback(MenuLayer *menu_layer, MenuIndex new_index, MenuIndex old_index, void *data)
{
  s_horiz_scroll_offset = 0;
  s_horiz_scroll_timer_timestamp = get_timestamp();
  return;

  // printf("selection changed %d -> %d (current = %d)", old_index.row, new_index.row, s_menu_selection_index);
  // MenuBrowser *browser = s_menu_browsers[s_browser_index];

  if (s_menu_selection_index == new_index.row)
  {
    // s_horiz_scroll_menu_reloading_to_scroll = true;
    return;
  }
  s_menu_selection_index = new_index.row;
  // s_horiz_scroll_offset = 0;

  // printf("updated current = %d, reloading to scroll? %s, timer active? %s",
  //   s_menu_selection_index,
  //   "NA",
  //   // s_horiz_scroll_menu_reloading_to_scroll ? "YES" : "NO",
  //   s_horiz_scroll_timer_active ? "YES" : "NO");

  // initiate_horiz_scroll_timer("menu_selection_changed_callback");

  /*
  if (s_horiz_scroll_menu_reloading_to_scroll)
  {
    s_horiz_scroll_menu_reloading_to_scroll = false;
  }
  else
  {
    initiate_horiz_scroll_timer("menu_selection_changed_callback");
  }*/
}

static void menu_select_long_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data)
{
  printf("long click!");
  // initiate_horiz_scroll_timer();
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data)
{
  printf("menu select");
  printf("browser index: %d", s_browser_index);
  MenuBrowser *browser = s_menu_browsers[s_browser_index];

  if (browser->menu_num_entries == 0)
  {
    return;
  }

  // if (s_horiz_scroll_timer_active)
  // {
  //   app_timer_cancel(s_horiz_scroll_timer);
  //   s_horiz_scroll_timer_active = false;
  // }
  s_horiz_scroll_offset = 0;

  s_horiz_scroll_menu_index = -1;
  s_menu_selection_index = -1;
  menu_layer_reload_data(browser->menu_layer);
  s_horiz_scroll_scrolling_still_required = false;

  printf("inside if");
  char *msg = browser->msg;
  char *route = browser->route;
  char *direction = browser->direction;
  char *stopid = browser->stopid;
  char *stopname = browser->stopname;
  char *extra = browser->extra;

  char *selector = browser->menu_selectors[cell_index->row];
  char *new_msg = NULL;
  char **split = NULL; // in case we need to split things... need to free afterward
  printf("starting strcmps w/ msg: %s", msg);

  if (strcmp(msg, MSG_ROUTES) == 0)
  {
    new_msg = MSG_DIRECTIONS;
    route = selector;
  }
  else if (strcmp(msg, MSG_DIRECTIONS) == 0)
  {
    new_msg = MSG_STOPS;
    direction = selector;
  }
  else if (strcmp(msg, MSG_STOPS) == 0)
  {
    new_msg = MSG_PREDICTIONS;
    stopid = selector;
    stopname = browser->menu_titles[cell_index->row];
  }
  else if (strcmp(msg, MSG_FAVORITES) == 0)
  {
    new_msg = MSG_PREDICTIONS;
    extra = selector;
  }
  else if (strcmp(msg, MSG_NEARBY_STOPS) == 0)
  {
    new_msg = MSG_PREDICTIONS;
    stopid = selector;
  }
  else if (strcmp(msg, MSG_NEARBY_ROUTES) == 0)
  {
    new_msg = MSG_PREDICTIONS;

    printf("selector: %s", selector);
    split = str_split(selector, '_');
    printf("done splitting!");
    if (split != NULL)
    {
      route = split[0];
      stopname = split[1];
      direction = split[2];
    }
  }
  else if (strcmp(msg, MSG_PREDICTIONS) == 0 || strcmp(msg, MSG_COMMUTE) == 0)
  {
    char *selector_copy = strdup(selector);
    split = str_split(selector_copy, '_');
    free(selector_copy);
    if (split != NULL)
    {
      route = split[0];
      direction = split[1];
      stopid = split[2];
      stopname = split[3];
    }
  }

  printf("lots'o stuff, new_msg: %s, route: %s", new_msg ? new_msg : "NULL", route ? route : "NULL");

  if (new_msg)
  {
    printf("msg: %s, route: %s, direction: %s, stopid: %s, stopname: %s: extra: %s",
    msg ? msg : "NULL",
    route ? route : "NULL",
    direction ? direction : "NULL",
    stopid ? stopid : "NULL",
    stopname ? stopname : "NULL",
    extra ? extra : "NULL");

    printf("browser index before: %d", s_browser_index);

    printf("browser index after: %d", s_browser_index);
    push_menu(new_msg, route, direction, stopid, stopname, extra);
  }
  else if (split != NULL)
  {
    push_settings_actionbar(route, direction, stopid, stopname);
  }

  if (split != NULL)
  {
    for (int i=0; split[i] != NULL; i++)
    {
      free(split[i]);
    }
    free(split);
  }
}

/*
 * APP MESSAGES
 */

static void inbox_received_callback(DictionaryIterator *iterator, void *context)
{
  // APP_LOG(APP_LOG_LEVEL_ERROR, "Inbox Received");

  MenuBrowser *browser = s_menu_browsers[s_browser_index];
  browser->loading_state = LOADING_STARTED;

  bool done = false;
  int num_entries = -1;
  int item_index = -1;
  char *title = NULL;
  char *subtitle = NULL;
  char *selector = NULL;
  char *msg_type = NULL;

  Tuple *t = dict_read_first(iterator);
  while (t != NULL)
  {
    switch(t->key)
    {
      case KEY_NUM_ENTRIES:
      {
        num_entries = (int)t->value->int32;
        break;
      }
      case KEY_ITEM_INDEX:
      {
        item_index = (int)t->value->int32;
        break;
      }
      case KEY_TITLES:
      {
        title = t->value->cstring;
        break;
      }
      case KEY_SELECTORS:
      {
        selector = t->value->cstring;
        break;
      }
      case KEY_SUBTITLES:
      {
        subtitle = t->value->cstring;
        break;
      }
      case KEY_MSG_TYPE:
      {
        msg_type = t->value->cstring;
        break;
      }
      case KEY_IS_FAVORITE:
      {
        browser->isfavorite = (int)t->value->int32 == 1;
        printf("setting is favorite: %s", browser->isfavorite ? "yes!" : "no!");
        break;
      }
      case KEY_IS_MORNING_COMMUTE:
      {
        browser->ismorningcommute = (int)t->value->int32 == 1;
        printf("setting is morning commute: %s", browser->ismorningcommute ? "yes!" : "no!");
        break;
      }
      case KEY_IS_EVENING_COMMUTE:
      {
        browser->iseveningcommute = (int)t->value->int32 == 1;
        printf("setting is evening commute: %s", browser->iseveningcommute ? "yes!" : "no!");
        break;
      }
    }

    if (strcmp("done", t->value->cstring) == 0)
    {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Done finding messages!");
      done = true;
      browser->loading_state = LOADING_DONE;
    }

    t = dict_read_next(iterator);
  }

  // we want to make sure we're creating space for entries
  // in the proper browser
  //
  // this is mostly to protect against wonkiness if a user hits
  // the back button before the first entry has loaded
  if (num_entries > 0 && strcmp(msg_type, browser->msg) == 0)
  {
    browser->menu_titles = calloc(num_entries, sizeof(char *));
    browser->menu_selectors = calloc(num_entries, sizeof(char *));
    browser->menu_subtitles = calloc(num_entries, sizeof(char *));

    browser->menu_title_heights = calloc(num_entries, sizeof(int));
    browser->menu_subtitle_heights = calloc(num_entries, sizeof(int));
  }

  Window *window = browser->menu_window;
  Layer *window_layer = window_get_root_layer(window);

  // if (browser->menu_num_entries > 0)
  if (done)
  {
    if (!browser->menu_layer)
    {
      GRect bounds = layer_get_frame(window_layer);

      MenuLayer *menu_layer = menu_layer_create(bounds);
      s_menu_browsers[s_browser_index]->menu_layer = menu_layer;

      menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks){
        .get_num_sections = menu_get_num_sections_callback,
        .get_num_rows = menu_get_num_rows_callback,
        .get_header_height = menu_get_header_height_callback,
        .get_cell_height = menu_get_cell_height_callback,
        .draw_header = menu_draw_header_callback,
        .draw_row = menu_draw_row_callback,
        .select_click = menu_select_callback,
        .select_long_click = menu_select_long_callback,
        .selection_changed = menu_selection_changed_callback
      });

      menu_layer_set_click_config_onto_window(menu_layer, window);

      #ifdef PBL_COLOR
        menu_layer_colorize(menu_layer);
      #endif

      layer_add_child(window_layer, menu_layer_get_layer(menu_layer));
    }
    // printf("scroll timer active? %s", s_horiz_scroll_timer_active ? "YES" : "NO");
    // if (s_horiz_scroll_timer_active)
    // {
    //   s_horiz_scroll_menu_reloading_to_scroll = true;
    // }
    menu_layer_reload_data(browser->menu_layer);

    layer_remove_from_parent(text_layer_get_layer(get_text_layer_loading(NULL)));
  }

  if (done)
  {
    printf("Done loading messages!");
    if (browser->menu_num_entries == 0)
    {
      setup_text_layer_noresults(window);
      layer_remove_child_layers(window_layer);
      layer_add_child(window_layer, text_layer_get_layer(s_text_layer_noresults));
    }
  }
  else // not done - keep going
  {
    MenuBrowser *onscreen_browser = browser;

    // if there's an errant message from the previous screen,
    // we need to catch it and place it in the CORRECT browser
    if (strcmp(msg_type, browser->msg) != 0)
    {
      printf("LOADING was interrupted w/ msg_type = %s, browser msg = %s",
             msg_type, browser->msg ? browser->msg : "null");
      browser = NULL;
      for (int msgIdx=0; msgIdx<s_browser_index; msgIdx++)
      {
        MenuBrowser *tmp_browser = s_menu_browsers[msgIdx];
        if (strcmp(msg_type, tmp_browser->msg) == 0)
        {
          printf("FOUND replacement at index = %d", msgIdx);
          browser = tmp_browser;
          break;
        }
      }
    }

    if (item_index != -1 && browser != NULL)
    {
      browser->menu_titles[item_index] = strdup(title);
      browser->menu_selectors[item_index] = strdup(selector);

      browser->menu_title_heights[item_index] = get_text_height(title, window_layer, FONT_KEY_GOTHIC_24_BOLD);
      browser->menu_subtitle_heights[item_index] = 0;

      if (subtitle != NULL)
      {
        browser->menu_subtitles[item_index] = strdup(subtitle);
        browser->menu_subtitle_heights[item_index] = get_text_height(subtitle, window_layer, FONT_KEY_GOTHIC_24);
      }

      if (item_index+1 > browser->menu_num_entries)
      {
        browser->menu_num_entries = item_index+1;
      }
      if (item_index == 0)
      {
        s_horiz_scroll_timer_timestamp = get_timestamp();

        if (s_browser_index == 0)
        {
          s_horiz_scroll_timer_active = true;
          s_horiz_scroll_timer = app_timer_register(HORIZ_SCROLL_WAIT_TIME, selected_index_monitor, NULL); // FIXME
        }

        // printf("calling initiate_horiz_scroll_timer");
        // initiate_horiz_scroll_timer("inbox_received_callback");
      }

      printf("idx: %d, title: %s, subt: %s, sel: %s",
        item_index,
        browser->menu_titles[item_index] ? browser->menu_titles[item_index] : "nil",
        browser->menu_subtitles[item_index] ? browser->menu_subtitles[item_index] : "nil",
        browser->menu_selectors[item_index] ? browser->menu_selectors[item_index] : "nil");
    }

    if (onscreen_browser == browser)
    {
      // send_menu_app_message(false); // TODO 2015-09-26: consider removing
    }
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context)
{
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context)
{
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context)
{
  // APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

/*
 * WINDOW MANAGEMENT
 */


static void initialize_browser(MenuBrowser *browser)
{
  browser->menu_window = NULL;
  browser->menu_layer = NULL;

  browser->menu_titles = NULL;
  browser->menu_subtitles = NULL;
  browser->menu_selectors = NULL;
  browser->menu_num_entries = 0;

  browser->loading_state = LOADING_NOT_STARTED;

  browser->msg = NULL;
  browser->route = NULL;
  browser->direction = NULL;
  browser->stopid = NULL;
  browser->stopname = NULL;
  browser->extra = NULL;
  browser->isfavorite = false;
}

static void free_browser_lists(MenuBrowser *browser)
{
  for (int i=0; i<browser->menu_num_entries; i++)
  {
    if (browser->menu_titles[i] != NULL)
    {
      free(browser->menu_titles[i]);
    }

    if (browser->menu_selectors[i] != NULL)
    {
      free(browser->menu_selectors[i]);
    }

    // subtitles are optional, so we need to check
    if (browser->menu_subtitles[i])
    {
      free(browser->menu_subtitles[i]);
    }
  }
  if (browser->menu_titles != NULL)
  {
    free(browser->menu_titles);
  }
  browser->menu_titles = NULL;
  if (browser->menu_subtitles != NULL)
  {
    free(browser->menu_subtitles);
  }
  browser->menu_subtitles = NULL;

  if (browser->menu_selectors != NULL)
  {
    free(browser->menu_selectors);
  }
  browser->menu_selectors = NULL;

  browser->menu_num_entries = 0;
}

static void window_load(Window *window)
{
  #ifdef PBL_COLOR
    window_colorize(window);
  #endif
  TextLayer *text_layer_loading = get_text_layer_loading(window);

  Layer *window_layer = window_get_root_layer(window);
  layer_add_child(window_layer, text_layer_get_layer(text_layer_loading));
}

static void window_unload(Window *window)
{
  // kill the timer so that we don't accidentally start sending
  // messages on a already-popped menu
  if (!s_timer_fired && s_timer != NULL)
  {
    app_timer_cancel(s_timer);
    s_timer_fired = false;
  }
  s_timer = NULL;


  MenuBrowser *browser = s_menu_browsers[s_browser_index];
  MenuLayer *menu_layer = browser->menu_layer;
  Window *menu_window = browser->menu_window;

  // Destroy the menu layer
  if (menu_layer != NULL)
  {
    menu_layer_destroy(menu_layer);
  }

  // Destroy the window
  if (menu_window != NULL)
  {
    window_destroy(menu_window);
  }

  free_browser_lists(browser);

  free(browser->msg);

  if (browser->route != NULL)
  {
    free(browser->route);
  }
  if (browser->direction != NULL)
  {
    free(browser->direction);
  }
  if (browser->stopid != NULL)
  {
    free(browser->stopid);
  }
  if (browser->stopname != NULL)
  {
    free(browser->stopname);
  }
  if (browser->extra != NULL)
  {
    free(browser->extra);
  }

  initialize_browser(browser); // set all to null
  free(browser);
  s_menu_browsers[s_browser_index] = NULL;

  if (s_browser_index > 0)
  {
    s_browser_index--;

    // the browser that'll soon be on the top of the stack
    MenuBrowser *backBrowser = s_menu_browsers[s_browser_index];
    if (backBrowser->loading_state != LOADING_DONE)
    {
      printf("back browser not finished loading!");
      menu_layer_reload_data(backBrowser->menu_layer);
      bool should_init = backBrowser->loading_state == LOADING_NOT_STARTED;
      send_menu_app_message(should_init, s_menu_browsers[s_browser_index]);
    }
    if (strcmp(backBrowser->msg, MSG_FAVORITES) == 0)
    {
      printf("returning to favorites menu!");
      free_browser_lists(backBrowser);

      layer_add_child(window_get_root_layer(backBrowser->menu_window),
        text_layer_get_layer(get_text_layer_loading(NULL)));
      send_menu_app_message(true, s_menu_browsers[s_browser_index]);
    }
  }
  else
  {
    if (s_horiz_scroll_timer_active && s_horiz_scroll_timer != NULL)
    {
      app_timer_cancel(s_horiz_scroll_timer);
      s_horiz_scroll_timer_active = false;
    }
    s_horiz_scroll_timer = NULL;

    free(s_menu_browsers);
    s_menu_browsers = NULL;

    destroy_text_layer_loading();
    text_layer_destroy(s_text_layer_noresults);
    s_text_layer_noresults = NULL;

    app_message_deregister_callbacks();
  }

  APP_LOG(APP_LOG_LEVEL_ERROR, "Unloaded");
}

void menu_browser_register_app_message_callbacks()
{
  printf("~ registered menu browser app msg callbacks");
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
}

void push_menu(char *msg, char *route, char *direction, char *stopid, char *stopname, char *extra)
{
  printf("menu push");
  if (s_menu_browsers == NULL)
  {
    printf("first push menu !");
    int num_browsers = 4;
    s_menu_browsers = calloc(num_browsers, sizeof(MenuBrowser *));
    s_browser_index = 0;
  }
  else
  {
    s_browser_index++;
  }

  menu_browser_register_app_message_callbacks();

  MenuBrowser *browser = calloc(1, sizeof(MenuBrowser)); // s_menu_browsers[s_browser_index];
  initialize_browser(browser);
  s_menu_browsers[s_browser_index] = browser;

  // printf("created browser: %p", browser);
  // for (int i=0; i<4; i++)
  // {
  //   printf("menu_browsers[%d] = %p", i, s_menu_browsers[i]);
  // }

  // Create main Window element and assign to pointer
  Window *window = window_create();
  browser->menu_window = window;

  // printf("created menu_window: %p", window);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(window, true);

  // Add a key-value pair
  browser->msg = strdup(msg);
  if (route != NULL)
  {
    browser->route = strdup(route);
  }
  if (direction != NULL)
  {
    browser->direction = strdup(direction);
  }
  if (stopid != NULL)
  {
    browser->stopid = strdup(stopid);
  }
  if (stopname != NULL)
  {
    browser->stopname = strdup(stopname);
  }
  if (extra != NULL)
  {
    browser->extra = strdup(extra);
  }
  // send_menu_app_message(true);

  printf("launching helper on timer");
  printf("browser index... %d", s_browser_index);
  // if (s_browser_index == 0)
  // send_menu_app_message(true); // FIXME
  s_timer_browser_index = s_browser_index;
  s_timer = app_timer_register(
    500,
    send_menu_app_message_helper,
    (void *)true);
}
