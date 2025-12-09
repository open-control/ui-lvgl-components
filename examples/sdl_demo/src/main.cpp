#include "lvgl.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_syswm.h>
#include "hal/hal.h"

#ifdef _WIN32
#include <dwmapi.h>
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#endif

#include <oc/ui/lvgl/widget/KnobWidget.hpp>
#include <oc/ui/lvgl/theme/BaseTheme.hpp>

using namespace oc::ui::lvgl;

#ifdef _WIN32
static void enable_dark_title_bar(SDL_Window* window) {
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (SDL_GetWindowWMInfo(window, &wmInfo)) {
        BOOL dark = TRUE;
        DwmSetWindowAttribute(wmInfo.info.win.window, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));
    }
}
#endif

static constexpr int WINDOW_W = 340;
static constexpr int WINDOW_H = 200;

static lv_obj_t* create_knob_with_label(lv_obj_t* parent, const char* name, uint8_t color_idx, bool centered = false) {
    // Container for knob + label
    lv_obj_t* cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_set_layout(cont, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_OVERFLOW_VISIBLE);

    // Knob
    static KnobWidget* knobs[8] = {nullptr};
    static int knob_count = 0;
    knobs[knob_count] = new KnobWidget(cont);
    knobs[knob_count]->colorIndex(color_idx);
    if (centered) knobs[knob_count]->centered(true);
    knobs[knob_count]->setValue(0.5f);
    knob_count++;

    // Label below
    lv_obj_t* label = lv_label_create(cont);
    lv_label_set_text(label, name);
    lv_obj_set_style_text_color(label, lv_color_hex(BaseTheme::Color::TEXT_PRIMARY), 0);

    return cont;
}

static void create_demo_ui(void);

int main(int argc, char **argv) {
    (void)argc; (void)argv;

    // DPI-aware: renders at true pixel size, no Windows scaling blur
    SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2");

    lv_init();
    lv_display_t* disp = sdl_hal_init(WINDOW_W, WINDOW_H);
    lv_sdl_window_set_resizeable(disp, true);

#ifdef _WIN32
    enable_dark_title_bar(lv_sdl_window_get_window(disp));
#endif

    create_demo_ui();

    while(1) {
        uint32_t time_till_next = lv_timer_handler();
        uint32_t delay = (time_till_next == LV_NO_TIMER_READY) ? 5 : LV_MIN(time_till_next, 5);
        SDL_Delay(delay);
    }
    return 0;
}

static void create_demo_ui(void) {
    lv_obj_t* scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_hex(BaseTheme::Color::BACKGROUND), 0);

    // Grid layout for knobs
    static int32_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static int32_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(scr, col_dsc, row_dsc);
    lv_obj_set_layout(scr, LV_LAYOUT_GRID);
    lv_obj_set_style_pad_all(scr, 16, 0);
    lv_obj_set_grid_align(scr, LV_GRID_ALIGN_SPACE_EVENLY, LV_GRID_ALIGN_CENTER);

    // Create knobs with different macro colors
    lv_obj_t* k1 = create_knob_with_label(scr, "Cutoff", 0);  // Red
    lv_obj_set_grid_cell(k1, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t* k2 = create_knob_with_label(scr, "Pan", 5, true);  // Blue, centered
    lv_obj_set_grid_cell(k2, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t* k3 = create_knob_with_label(scr, "Reso", 2);  // Yellow
    lv_obj_set_grid_cell(k3, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t* k4 = create_knob_with_label(scr, "Attack", 3);  // Green
    lv_obj_set_grid_cell(k4, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 0, 1);
}
