#include <pebble.h>
#include "menu_browser.h"
#include "app_colors.h"
// #include "str_split.h"

#define NUM_MENU_ICONS 2

static Window *s_main_window;
static MenuLayer *s_menu_layer;

static GBitmap *s_menu_icons[NUM_MENU_ICONS];
static GBitmap *s_menu_icons_highlighted[NUM_MENU_ICONS];

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data)
{
  return 1;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data)
{
  switch (section_index)
  {
    case 0:
      return 4;
    default:
      return 0;
  }
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data)
{
  return 0; // MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data)
{
  // Determine which section we're working with
  /*
  switch (section_index) {
    case 0:
      // Draw title text in the section header
      menu_cell_basic_header_draw(ctx, cell_layer, "Some example items");
      break;
    case 1:
      menu_cell_basic_header_draw(ctx, cell_layer, "One more");
      break;
  }
  */
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data)
{
  // Determine which section we're going to draw in
  switch (cell_index->section)
  {
    case 0:
      // Use the row to specify which item we'll draw
      switch (cell_index->row)
      {
        case 0:
        {
          menu_cell_basic_draw(ctx, cell_layer, "Commute", NULL, NULL);
          break;
        }
        case 1:
        {
          // This is a basic menu item with a title and subtitle
          GBitmap *icon = menu_cell_layer_is_highlighted(cell_layer) ? s_menu_icons_highlighted[0] : s_menu_icons[0];
          menu_cell_basic_draw(ctx, cell_layer, "Favorites", NULL, icon);
          break;
        }
        case 2:
        {
          GBitmap *icon = menu_cell_layer_is_highlighted(cell_layer) ? s_menu_icons_highlighted[1] : s_menu_icons[1];
          menu_cell_basic_draw(ctx, cell_layer, "Routes", NULL, icon);
          // menu_cell_basic_draw(ctx, cell_layer, "Icon Item", "Select to cycle", s_menu_icons[s_current_icon]);
          break;
        }
        case 3:
        {
          menu_cell_basic_draw(ctx, cell_layer, "Location", NULL, NULL);
          break;
        }
      }
      break;
    default:
      break;
  }
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data)
{
  // Use the row to specify which item will receive the select action
  switch (cell_index->row)
  {
    case 0:
    {
      push_menu("getcommute", NULL, NULL, NULL, NULL, NULL);
      break;
    }

    // Favorites
    case 1:
    {
      push_menu("getfavorites", NULL, NULL, NULL, NULL, NULL);
      break;
    }

    // Routes
    case 2:
    {
      // push_routes();
      push_menu("getroutes", NULL, NULL, NULL, NULL, NULL);
      break;
    }

    // Location
    case 3:
    {
      push_menu("getnearbystops", NULL, NULL, NULL, NULL, NULL);
      break;
    }
  }
}

static void main_window_load(Window *window)
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

  // s_menu_icons[0] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BUS);
  s_menu_icons[0] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STAR);
  s_menu_icons[1] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BUS);

  s_menu_icons_highlighted[0] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STAR_HIGHLIGHTED);
  s_menu_icons_highlighted[1] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BUS_HIGHLIGHTED);

  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void main_window_unload(Window *window)
{
  // Destroy the menu layer
  menu_layer_destroy(s_menu_layer);

  for (int i = 0; i < NUM_MENU_ICONS; i++)
  {
    gbitmap_destroy(s_menu_icons[i]);
    gbitmap_destroy(s_menu_icons_highlighted[i]);
  }
}

static void init()
{
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
}

static void deinit()
{
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void)
{
  init();
  app_event_loop();
  deinit();
}
