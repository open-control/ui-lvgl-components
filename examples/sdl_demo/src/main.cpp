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

#include <algorithm>
#include <cmath>

#include <oc/ui/lvgl/component/ParameterKnob.hpp>
#include <oc/ui/lvgl/component/ParameterEnum.hpp>
#include <oc/ui/lvgl/component/ParameterSwitch.hpp>
#include <oc/ui/lvgl/widget/StateIndicator.hpp>
#include <oc/ui/lvgl/widget/Label.hpp>
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

static constexpr int WINDOW_W = 600;
static constexpr int WINDOW_H = 160;
static constexpr int CELL_W = 80;
static constexpr int CELL_H = 100;

// ============================================================================
// Component storage
// ============================================================================
static ParameterKnob* knob_params[4] = {nullptr};
static ParameterEnum* wave_param = nullptr;
static ParameterSwitch* bypass_param = nullptr;
static StateIndicator* indicators[3] = {nullptr};
static Label* status_label = nullptr;  // Label for indicators column

// ListItem value cycling
static const char* wave_values[] = {"Sine", "Triangle", "Sawtooth", "Square", "Noise"};
static int wave_index = 0;

// Knob drag state (desktop interaction)
struct KnobDragState {
    KnobWidget* widget = nullptr;
    lv_coord_t start_y = 0;
};
static KnobDragState knob_drag;

// ============================================================================
// Desktop interactions (external to widgets - for demo only)
// ============================================================================

// Knob drag handler
static void knob_drag_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    auto* knob = static_cast<KnobWidget*>(lv_event_get_user_data(e));

    if (code == LV_EVENT_PRESSED) {
        lv_point_t p;
        lv_indev_get_point(lv_indev_active(), &p);
        knob_drag.widget = knob;
        knob_drag.start_y = p.y;
    }
    else if (code == LV_EVENT_PRESSING && knob_drag.widget == knob) {
        lv_point_t p;
        lv_indev_get_point(lv_indev_active(), &p);
        float delta = (knob_drag.start_y - p.y) / 100.0f;
        knob_drag.start_y = p.y;

        float new_value = knob->getValue() + delta;
        new_value = std::clamp(new_value, 0.0f, 1.0f);
        knob->setValue(new_value);

        // Knob 0 controls indicator 0
        if (knob == &knob_params[0]->knob() && indicators[0]) {
            indicators[0]->setState(new_value > 0.5f ? StateIndicator::State::ACTIVE
                                                     : StateIndicator::State::OFF);
        }
    }
    else if (code == LV_EVENT_RELEASED) {
        knob_drag.widget = nullptr;
    }
}

// Button click handler
static void button_click_cb(lv_event_t* e) {
    (void)e;
    if (bypass_param) {
        bool new_state = !bypass_param->button().getState();
        bypass_param->button().setState(new_state);
        bypass_param->button().setText(new_state ? "ON" : "OFF");
    }
}

// ListItem click handler (click to cycle values)
static void list_click_cb(lv_event_t* e) {
    (void)e;
    wave_index = (wave_index + 1) % 5;

    if (wave_param) {
        wave_param->valueLabel().setText(wave_values[wave_index]);
        wave_param->enumWidget().triggerFlash();
    }
}

// ============================================================================
// Component creation helpers
// ============================================================================

static void setup_knob_interaction(KnobWidget& knob) {
    lv_obj_t* obj = knob.getElement();
    lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(obj, knob_drag_cb, LV_EVENT_PRESSED, &knob);
    lv_obj_add_event_cb(obj, knob_drag_cb, LV_EVENT_PRESSING, &knob);
    lv_obj_add_event_cb(obj, knob_drag_cb, LV_EVENT_RELEASED, &knob);
}

// ============================================================================
// Main
// ============================================================================

static void create_demo_ui(void);

int main(int argc, char **argv) {
    (void)argc; (void)argv;

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

    // Grid: 7 columns fixed width, 1 row fixed height
    static int32_t col_dsc[] = {
        CELL_W, CELL_W, CELL_W, CELL_W, CELL_W, CELL_W, CELL_W,
        LV_GRID_TEMPLATE_LAST
    };
    static int32_t row_dsc[] = {CELL_H, LV_GRID_TEMPLATE_LAST};

    lv_obj_set_grid_dsc_array(scr, col_dsc, row_dsc);
    lv_obj_set_layout(scr, LV_LAYOUT_GRID);
    lv_obj_set_style_pad_all(scr, 16, 0);
    lv_obj_set_style_pad_column(scr, 8, 0);
    lv_obj_set_grid_align(scr, LV_GRID_ALIGN_SPACE_EVENLY, LV_GRID_ALIGN_CENTER);

    // ========================================================================
    // ParameterKnob components
    // ========================================================================

    // Knob 0 - Cutoff
    knob_params[0] = new ParameterKnob(scr);
    knob_params[0]->knob()
        .trackColor(BaseTheme::Color::getMacroColor(0))
        .flashColor(BaseTheme::Color::ACTIVE);
    knob_params[0]->knob().setValue(0.5f);
    knob_params[0]->label()
        .alignment(LV_TEXT_ALIGN_CENTER)
        .autoScroll(true);
    knob_params[0]->label().setText("Cutoff Frequency");  // Long - will autoscroll
    setup_knob_interaction(knob_params[0]->knob());
    lv_obj_set_grid_cell(knob_params[0]->getElement(), LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    // Knob 1 - Pan (centered)
    knob_params[1] = new ParameterKnob(scr);
    knob_params[1]->knob()
        .trackColor(BaseTheme::Color::getMacroColor(5))
        .flashColor(BaseTheme::Color::ACTIVE)
        .centered(true);
    knob_params[1]->knob().setValue(0.5f);
    knob_params[1]->label()
        .alignment(LV_TEXT_ALIGN_CENTER);
    knob_params[1]->label().setText("Pan");
    setup_knob_interaction(knob_params[1]->knob());
    lv_obj_set_grid_cell(knob_params[1]->getElement(), LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    // Knob 2 - Resonance
    knob_params[2] = new ParameterKnob(scr);
    knob_params[2]->knob()
        .trackColor(BaseTheme::Color::getMacroColor(2))
        .flashColor(BaseTheme::Color::ACTIVE);
    knob_params[2]->knob().setValue(0.5f);
    knob_params[2]->label()
        .alignment(LV_TEXT_ALIGN_CENTER);
    knob_params[2]->label().setText("Reso");
    setup_knob_interaction(knob_params[2]->knob());
    lv_obj_set_grid_cell(knob_params[2]->getElement(), LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    // Knob 3 - Attack
    knob_params[3] = new ParameterKnob(scr);
    knob_params[3]->knob()
        .trackColor(BaseTheme::Color::getMacroColor(3))
        .flashColor(BaseTheme::Color::ACTIVE);
    knob_params[3]->knob().setValue(0.5f);
    knob_params[3]->label()
        .alignment(LV_TEXT_ALIGN_CENTER);
    knob_params[3]->label().setText("Attack");
    setup_knob_interaction(knob_params[3]->knob());
    lv_obj_set_grid_cell(knob_params[3]->getElement(), LV_GRID_ALIGN_STRETCH, 3, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    // ========================================================================
    // ParameterEnum component (Wave selector)
    // ========================================================================
    wave_param = new ParameterEnum(scr);
    wave_param->enumWidget()
        .lineColor(BaseTheme::Color::getMacroColor(4))
        .flashColor(BaseTheme::Color::ACTIVE);
    wave_param->valueLabel()
        .alignment(LV_TEXT_ALIGN_CENTER)
        .autoScroll(true);
    wave_param->valueLabel().setText(wave_values[wave_index]);
    wave_param->nameLabel()
        .alignment(LV_TEXT_ALIGN_CENTER);
    wave_param->nameLabel().setText("Wave");

    // Click on the whole component container
    lv_obj_add_flag(wave_param->getElement(), LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(wave_param->getElement(), list_click_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_set_grid_cell(wave_param->getElement(), LV_GRID_ALIGN_STRETCH, 4, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    // ========================================================================
    // ParameterSwitch component (Bypass)
    // ========================================================================
    bypass_param = new ParameterSwitch(scr);
    bypass_param->button()
        .offColor(BaseTheme::Color::INACTIVE)
        .onColor(BaseTheme::Color::getMacroColor(1))
        .textOffColor(BaseTheme::Color::TEXT_PRIMARY)
        .textOnColor(BaseTheme::Color::TEXT_PRIMARY_INVERTED);
    bypass_param->button().setText("OFF");
    bypass_param->label()
        .alignment(LV_TEXT_ALIGN_CENTER);
    bypass_param->label().setText("Bypass");

    lv_obj_t* btn_inner = bypass_param->button().inner();
    lv_obj_add_flag(btn_inner, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(btn_inner, button_click_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_set_grid_cell(bypass_param->getElement(), LV_GRID_ALIGN_STRETCH, 5, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    // ========================================================================
    // Indicators container (no component - standalone widgets)
    // ========================================================================
    lv_obj_t* ind_container = lv_obj_create(scr);
    lv_obj_set_size(ind_container, lv_pct(100), lv_pct(100));  // Fill grid cell
    lv_obj_set_style_bg_opa(ind_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(ind_container, 0, 0);
    lv_obj_set_style_pad_all(ind_container, 0, 0);
    lv_obj_set_layout(ind_container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(ind_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ind_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(ind_container, 6, 0);
    lv_obj_set_style_pad_top(ind_container, 10, 0);
    lv_obj_set_scrollbar_mode(ind_container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_grid_cell(ind_container, LV_GRID_ALIGN_STRETCH, 6, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    // Indicator 0 - linked to Knob 0
    indicators[0] = new StateIndicator(ind_container, 12);
    indicators[0]->color(StateIndicator::State::OFF, BaseTheme::Color::getMacroColor(0))
                 .color(StateIndicator::State::ACTIVE, BaseTheme::Color::getMacroColor(0))
                 .opacity(StateIndicator::State::OFF, LV_OPA_40)
                 .opacity(StateIndicator::State::ACTIVE, LV_OPA_COVER);
    indicators[0]->setState(StateIndicator::State::ACTIVE);

    // Indicator 1
    indicators[1] = new StateIndicator(ind_container, 12);
    indicators[1]->color(StateIndicator::State::OFF, BaseTheme::Color::getMacroColor(3))
                 .color(StateIndicator::State::ACTIVE, BaseTheme::Color::getMacroColor(3))
                 .opacity(StateIndicator::State::OFF, LV_OPA_40)
                 .opacity(StateIndicator::State::ACTIVE, LV_OPA_COVER);
    indicators[1]->setState(StateIndicator::State::ACTIVE);

    // Indicator 2
    indicators[2] = new StateIndicator(ind_container, 12);
    indicators[2]->color(StateIndicator::State::OFF, BaseTheme::Color::getMacroColor(5))
                 .color(StateIndicator::State::ACTIVE, BaseTheme::Color::getMacroColor(5))
                 .opacity(StateIndicator::State::OFF, LV_OPA_40)
                 .opacity(StateIndicator::State::ACTIVE, LV_OPA_COVER);
    indicators[2]->setState(StateIndicator::State::OFF);

    // Status label for indicators column
    status_label = new Label(ind_container);
    status_label->color(BaseTheme::Color::TEXT_PRIMARY)
                .alignment(LV_TEXT_ALIGN_CENTER);
    lv_obj_set_width(status_label->getElement(), lv_pct(100));
    status_label->setText("Status");
}
