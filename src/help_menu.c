#include "pebble.h"
#include "app_colors.h"
#include "str_utils.h"

static Window *s_help_window = NULL;
static Window *s_detail_window = NULL;

static MenuLayer *s_menu_layer = NULL;

static ScrollLayer *s_detail_scroll_layer = NULL;
static TextLayer *s_detail_text_layer = NULL;
static TextLayer *s_detail_title_text_layer = NULL;

static char *s_detail_text = NULL;
static char *s_detail_title = NULL;

//
// Detail View
//

static TextLayer *detail_text_layer_create(GRect bounds, char *text, bool is_title)
{
  TextLayer *text_layer = text_layer_create(bounds);
  text_layer_set_text(text_layer, text);

  if (is_title)
  {
    text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  }
  else
  {
    text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_alignment(text_layer, GTextAlignmentLeft);
  }

  return text_layer;
}

static void detail_window_load(Window *window)
{
  #ifdef PBL_COLOR
    window_colorize(window);
  #endif

  // set up text layer
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  int16_t content_width = bounds.size.w-10;

  // Initialize the text layers
  GRect max_title_bounds = GRect(5, 0, content_width, 2000);
  s_detail_title_text_layer = detail_text_layer_create(max_title_bounds, s_detail_title, true);
  GSize title_size = text_layer_get_content_size(s_detail_title_text_layer);
  title_size.w = content_width;

  GRect max_text_bounds = GRect(5, title_size.h+2, content_width, 2000);
  s_detail_text_layer = detail_text_layer_create(max_text_bounds, s_detail_text, false);

  // Initialize the scroll layer
  s_detail_scroll_layer = scroll_layer_create(bounds);

  // This binds the scroll layer to the window so that up and down map to scrolling
  // You may use scroll_layer_set_callbacks to add or override interactivity
  scroll_layer_set_click_config_onto_window(s_detail_scroll_layer, window);

  // Add the layers for display

  scroll_layer_add_child(s_detail_scroll_layer, text_layer_get_layer(s_detail_text_layer));
  scroll_layer_add_child(s_detail_scroll_layer, text_layer_get_layer(s_detail_title_text_layer));

  layer_add_child(window_layer, scroll_layer_get_layer(s_detail_scroll_layer));

  // Trim text layer and scroll content to fit text box
  // GSize title_size = text_layer_get_content_size(s_detail_title_text_layer);
  title_size.w = content_width;
  title_size.h += 5;
  GSize text_size = text_layer_get_content_size(s_detail_text_layer);
  text_size.h += title_size.h;
  text_size.w = content_width;

  text_layer_set_size(s_detail_title_text_layer, title_size);
  text_layer_set_size(s_detail_text_layer, text_size);

  scroll_layer_set_content_size(s_detail_scroll_layer, GSize(bounds.size.w, title_size.h + text_size.h));
}

static void detail_window_unload(Window *window)
{
  text_layer_destroy(s_detail_title_text_layer);
  s_detail_title_text_layer = NULL;
  text_layer_destroy(s_detail_text_layer);
  s_detail_text_layer = NULL;
  scroll_layer_destroy(s_detail_scroll_layer);
  s_detail_scroll_layer = NULL;

  window_destroy(s_detail_window);
  s_detail_window = NULL;

  free(s_detail_title);
  s_detail_title = NULL;
  free(s_detail_text);
  s_detail_text = NULL;
}

static void push_detail_view(char *title, char *text)
{
  s_detail_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_detail_window, (WindowHandlers) {
    .load = detail_window_load,
    .unload = detail_window_unload
  });

  s_detail_title = strdup(title);
  s_detail_text = strdup(text);

  // Show the Window on the watch, with animated=true
  window_stack_push(s_detail_window, true);
}

//
// HELP MENU FUNCTIONS
//

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data)
{
  return 1;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data)
{
  switch (section_index)
  {
    case 0:
      return 5;
    default:
      return 0;
  }
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data)
{
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data)
{
  switch (section_index) {
    case 0:
      menu_cell_basic_header_draw(ctx, cell_layer, "Help Menu");
      break;
  }
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data)
{
  char *title = NULL;

  // Determine which section we're going to draw in
  switch (cell_index->section)
  {
    case 0:
    {
      // Use the row to specify which item we'll draw
      switch (cell_index->row)
      {
        case 0:
        {
          title = "Overview";
          break;
        }
        case 1:
        {
          title = "Navigate the App";
          break;
        }
        case 2:
        {
          title = "Your Commute";
          break;
        }
        case 3:
        {
          title = "Your Favorites";
          break;
        }
        case 4:
        {
          title = "Using Location";
        }
      }
    }
    default:
    {
      break;
    }
  }

  if (title != NULL)
  {
    menu_cell_basic_draw(ctx, cell_layer, title, NULL, NULL);
  }
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data)
{
  // Use the row to specify which item will receive the select action
  switch (cell_index->row)
  {
    // Overview
    case 0:
    {
      push_detail_view("Overview",
      "Welcome! This watchapp gives you realtime updates to busses in Pittsburgh, Pennsylvania.\n\n"
      "You can navigate through all available routes or grab those based on your location to find your bus.\n\n"
      "Once you've found your bus (at a particular stop), you can add it to your favorites or your morning and evening commute");
      break;
    }

    // Navigation
    case 1:
    {
      push_detail_view("Navigation",
      "If you know the bus you're looking for, select Routes from the main menu. "
      "Then select your route (e.g., P1), the direction you're headed, and the stop "
      "you're at. The screen will then show you all bus predictions at that stop.\n\n"
      "If you're near your bus stop, select Location from the main menu. This will use GPS "
      "to locate nearby bus stops, then go through much the same process as above.\n\n"
      "FYI: you may see more than just your bus on the Predictions screen, but the route "
      "you selected will be sorted first. For example, if you're looking for P1s, you might "
      "also see P2 and P71 listings, but they'll be after the P1 predictions.");
      break;
    }

    // Commute
    case 2:
    {
      push_detail_view("Commute",
      "Commute is your one-stop screen for getting to and from work every day.\n\n"
      "Add a bus to your commute by selecting an entry on the Predictions screen. "
      "Then select UP to add to your morning commute, DOWN to add to your evening commute.\n\n"
      "To view busses on your commute, select Commute from the main menu.");
      break;
    }

    // Favorites
    case 3:
    {
      push_detail_view("Favorites",
      "Favorites are a quick reference to common routes. Add them by selecting an entry "
      "on the Predictions screen and then hit the middle button.\n\n"
      "To view your favorites, select Favorites from the main menu.");
      break;
    }

    // Location
    case 4:
    {
      push_detail_view("Location",
      "This option uses GPS to locate nearby busses (described in more depth "
      "in Navigation help). I'd just like to note that all of the GPS data is "
      "handled LOCALLY, on the watch and phone, so none of your location information "
      "is sent to any server. I built this with your privacy in mind!");
      break;
    }
  }
}

//
// WINDOW LOADING / UNLOADING
//

static void help_window_load(Window *window)
{
  // Now we prepare to initialize the menu layer
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  #ifdef PBL_COLOR
    window_colorize(window);
  #endif

  // Create the menu layer
  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = menu_get_header_height_callback,
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
  });

  #ifdef PBL_COLOR
    // menu_layer_pad_bottom_enable(s_menu_layer, true);
    menu_layer_colorize(s_menu_layer);
  #endif

  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(s_menu_layer, window);

  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void help_window_unload(Window *window)
{
  window_destroy(s_help_window);
  s_help_window = NULL;
}

void push_help_menu()
{
  // Create main Window element and assign to pointer
  s_help_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_help_window, (WindowHandlers) {
    .load = help_window_load,
    .unload = help_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_help_window, true);
}
