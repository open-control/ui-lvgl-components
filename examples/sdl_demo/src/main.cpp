#include "lvgl.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_syswm.h>
#include "hal/hal.h"
#include "hw_simulator/HwSimulator.hpp"
#include "SDL2_gfxPrimitives.h"

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

// Hardware simulator (full panel)
static constexpr int PANEL_SIZE = HwLayout::PANEL_SIZE;  // 1013px (correct proportions)

// LVGL screen size (embedded display)
static constexpr int SCREEN_W = HwLayout::SCREEN_W;  // 320px
static constexpr int SCREEN_H = HwLayout::SCREEN_H;  // 240px

// Hardware simulator instance
static HwSimulator hwSim;


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

// Hardware event handlers
static void on_hw_button(int id, bool pressed) {
    printf("Button %d %s\n", id, pressed ? "pressed" : "released");

    // Example: Link hardware button to LVGL actions
    if (id == HwId::BOTTOM_CENTER && pressed && bypass_param) {
        bool new_state = !bypass_param->button().getState();
        bypass_param->button().setState(new_state);
        bypass_param->button().setText(new_state ? "ON" : "OFF");
    }
}

static void on_hw_encoder(int id, float value) {
    printf("Encoder %d value: %.3f\n", id, value);

    // Link macro encoders to LVGL knobs
    int macroIndex = id - HwId::MACRO_ENC_1;
    if (macroIndex >= 0 && macroIndex < 4 && knob_params[macroIndex]) {
        knob_params[macroIndex]->knob().setValue(value);
    }
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;

    // Platform-specific SDL hints (must be set before SDL_Init)
#ifdef _WIN32
    SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2");
#elif defined(__linux__)
    SDL_SetHint(SDL_HINT_VIDEO_WAYLAND_PREFER_LIBDECOR, "1");
    SDL_SetHint(SDL_HINT_VIDEO_WAYLAND_ALLOW_LIBDECOR, "1");
#endif

    lv_init();

    // Create LVGL display at PANEL size (square - no legend, indicator is inside panel)
    lv_display_t* disp = sdl_hal_init(PANEL_SIZE, PANEL_SIZE);
    lv_sdl_window_set_title(disp, "Hardware Simulator");

    // Get SDL window and renderer from LVGL
    SDL_Window* window = lv_sdl_window_get_window(disp);
    SDL_Renderer* renderer = static_cast<SDL_Renderer*>(lv_sdl_window_get_renderer(disp));
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

#ifdef _WIN32
    enable_dark_title_bar(window);
#endif

    // Initialize hardware simulator
    hwSim.init(renderer);
    hwSim.setButtonCallback(on_hw_button);
    hwSim.setEncoderCallback(on_hw_encoder);

    // Create LVGL UI
    create_demo_ui();

    // Get screen rect for LVGL positioning
    SDL_Rect screenRect = hwSim.getScreenRect();

    // Create render target texture for LVGL compositing
    // This allows us to capture LVGL's rendering and composite it with hwSim
    SDL_Texture* lvglTexture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        PANEL_SIZE, PANEL_SIZE
    );
    SDL_SetTextureBlendMode(lvglTexture, SDL_BLENDMODE_BLEND);

    // Main loop
    while(1) {
        // Handle SDL events (for hardware simulation)
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                SDL_DestroyTexture(lvglTexture);
                return 0;
            }

            // Adjust mouse coordinates for LVGL (offset by screen position)
            if (event.type == SDL_MOUSEBUTTONDOWN ||
                event.type == SDL_MOUSEBUTTONUP ||
                event.type == SDL_MOUSEMOTION) {
                // Check if inside LVGL screen area
                int mx = (event.type == SDL_MOUSEMOTION) ? event.motion.x : event.button.x;
                int my = (event.type == SDL_MOUSEMOTION) ? event.motion.y : event.button.y;

                if (mx >= screenRect.x && mx < screenRect.x + screenRect.w &&
                    my >= screenRect.y && my < screenRect.y + screenRect.h) {
                    // Inside LVGL area - let LVGL handle it (coordinates will be wrong though)
                    // For now, hardware sim won't capture these events
                } else {
                    // Outside LVGL area - hardware simulator handles it
                    hwSim.handleEvent(event);
                }
            } else if (event.type == SDL_MOUSEWHEEL) {
                // Check mouse position for wheel events
                int mx, my;
                SDL_GetMouseState(&mx, &my);
                if (!(mx >= screenRect.x && mx < screenRect.x + screenRect.w &&
                      my >= screenRect.y && my < screenRect.y + screenRect.h)) {
                    hwSim.handleEvent(event);
                }
            }
        }

        // Redirect LVGL rendering to our texture
        // This way LVGL's internal SDL_RenderPresent won't show an incomplete frame
        SDL_SetRenderTarget(renderer, lvglTexture);

        // LVGL timer handler renders to our texture
        // Its internal SDL_RenderPresent will present the previous frame (unchanged)
        uint32_t time_till_next = lv_timer_handler();
        (void)time_till_next;

        // Switch back to rendering to window
        SDL_SetRenderTarget(renderer, NULL);

        // Clear and draw hwSim first (background)
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        hwSim.render();

        // Composite ONLY the LVGL screen area on top (not the full texture which has black background)
        // This preserves hwSim in the areas outside the LVGL screen
        SDL_Rect lvglScreenRect = {
            HwLayout::SCREEN_X, HwLayout::SCREEN_Y,
            HwLayout::SCREEN_W, HwLayout::SCREEN_H
        };
        SDL_RenderCopy(renderer, lvglTexture, &lvglScreenRect, &lvglScreenRect);

        // Single present with everything composited
        SDL_RenderPresent(renderer);

        // Small delay
        SDL_Delay(1);
    }
    return 0;
}

static void create_demo_ui(void) {
    // Layout constants (match production)
    constexpr lv_coord_t TOP_BAR_HEIGHT = 20;
    constexpr lv_coord_t TRANSPORT_BAR_HEIGHT = 20;
    constexpr int GRID_COLS = 4;
    constexpr int GRID_ROWS = 2;

    lv_obj_t* scr = lv_screen_active();
    // Make screen transparent so hardware sim shows through
    lv_obj_set_style_bg_opa(scr, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(scr, 0, 0);

    // ========================================================================
    // Screen container - positioned at the "display" area of the hardware
    // ========================================================================
    lv_obj_t* screen_container = lv_obj_create(scr);
    lv_obj_set_pos(screen_container, HwLayout::SCREEN_X, HwLayout::SCREEN_Y);
    lv_obj_set_size(screen_container, HwLayout::SCREEN_W, HwLayout::SCREEN_H);
    lv_obj_set_style_bg_color(screen_container, lv_color_hex(base_theme::color::BACKGROUND), 0);
    lv_obj_set_style_border_width(screen_container, 0, 0);
    lv_obj_set_style_pad_all(screen_container, 0, 0);
    lv_obj_set_style_radius(screen_container, 0, 0);
    lv_obj_set_scrollbar_mode(screen_container, LV_SCROLLBAR_MODE_OFF);

    // Main layout inside screen container (flex column)
    lv_obj_set_layout(screen_container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(screen_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_gap(screen_container, 0, 0);

    // ========================================================================
    // TopBar (20px height)
    // ========================================================================
    lv_obj_t* top_bar = lv_obj_create(screen_container);
    lv_obj_set_size(top_bar, LV_PCT(100), TOP_BAR_HEIGHT);
    lv_obj_set_style_bg_color(top_bar, lv_color_hex(base_theme::color::BACKGROUND), 0);
    lv_obj_set_style_border_width(top_bar, 0, 0);
    lv_obj_set_style_pad_all(top_bar, 0, 0);
    lv_obj_set_scrollbar_mode(top_bar, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t* title_label = lv_label_create(top_bar);
    lv_label_set_text(title_label, "UI Components Demo");
    lv_obj_set_style_text_color(title_label, lv_color_hex(base_theme::color::TEXT_SECONDARY), 0);
    lv_obj_center(title_label);

    // ========================================================================
    // Body container (takes remaining space, grid 4x2)
    // ========================================================================
    lv_obj_t* body = lv_obj_create(screen_container);
    lv_obj_set_size(body, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_grow(body, 1);
    lv_obj_set_style_bg_opa(body, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(body, 0, 0);
    lv_obj_set_style_pad_all(body, 0, 0);
    lv_obj_set_scrollbar_mode(body, LV_SCROLLBAR_MODE_OFF);

    // Grid layout: 4 columns, 2 rows
    static int32_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static int32_t row_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(body, col_dsc, row_dsc);
    lv_obj_set_layout(body, LV_LAYOUT_GRID);
    lv_obj_set_style_pad_column(body, 0, 0);
    lv_obj_set_style_pad_row(body, 0, 0);

    // ========================================================================
    // 8 ParameterKnob components in 4x2 grid
    // ========================================================================
    const char* knob_names[8] = {
        "Macro 1", "Macro 2", "Macro 3", "Macro 4",
        "Macro 5", "Macro 6", "Macro 7", "Macro 8"
    };
    bool knob_centered[8] = {false, false, false, false, true, false, false, false};  // Macro 5 centered

    for (int i = 0; i < 4; i++) {
        int col = i % GRID_COLS;
        int row = i / GRID_COLS;

        knob_params[i] = new ParameterKnob(body);
        knob_params[i]->knob()
            .trackColor(base_theme::color::getMacroColor(i))
            .flashColor(base_theme::color::ACTIVE);
        if (knob_centered[i]) {
            knob_params[i]->knob().centered(true);
        }
        knob_params[i]->knob().setValue(0.5f);
        knob_params[i]->label().alignment(LV_TEXT_ALIGN_CENTER);
        knob_params[i]->label().setText(knob_names[i]);
        setup_knob_interaction(knob_params[i]->knob());

        lv_obj_set_grid_cell(knob_params[i]->getElement(),
            LV_GRID_ALIGN_STRETCH, col, 1,
            LV_GRID_ALIGN_STRETCH, row, 1);
    }

    // Row 2: ParameterEnum, ParameterSwitch, and 2 more knobs
    // Slot 4 - Wave selector (ParameterEnum)
    wave_param = new ParameterEnum(body);
    wave_param->enumWidget()
        .lineColor(base_theme::color::getMacroColor(4))
        .flashColor(base_theme::color::ACTIVE);
    wave_param->valueLabel().alignment(LV_TEXT_ALIGN_CENTER).autoScroll(true);
    wave_param->valueLabel().setText(wave_values[wave_index]);
    wave_param->nameLabel().alignment(LV_TEXT_ALIGN_CENTER);
    wave_param->nameLabel().setText("Wave");
    lv_obj_add_flag(wave_param->getElement(), LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(wave_param->getElement(), list_click_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_set_grid_cell(wave_param->getElement(), LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    // Slot 5 - Bypass (ParameterSwitch)
    bypass_param = new ParameterSwitch(body);
    bypass_param->button()
        .offColor(base_theme::color::INACTIVE)
        .onColor(base_theme::color::getMacroColor(5))
        .textOffColor(base_theme::color::TEXT_PRIMARY)
        .textOnColor(base_theme::color::TEXT_PRIMARY_INVERTED);
    bypass_param->button().setText("OFF");
    bypass_param->label().alignment(LV_TEXT_ALIGN_CENTER);
    bypass_param->label().setText("Bypass");
    lv_obj_t* btn_inner = bypass_param->button().inner();
    lv_obj_add_flag(btn_inner, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(btn_inner, button_click_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_set_grid_cell(bypass_param->getElement(), LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    // Slot 6 & 7 - Indicators column
    lv_obj_t* ind_container = lv_obj_create(body);
    lv_obj_set_style_bg_opa(ind_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(ind_container, 0, 0);
    lv_obj_set_style_pad_all(ind_container, 0, 0);
    lv_obj_set_layout(ind_container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(ind_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ind_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(ind_container, 6, 0);
    lv_obj_set_scrollbar_mode(ind_container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_grid_cell(ind_container, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    for (int i = 0; i < 3; i++) {
        indicators[i] = new StateIndicator(ind_container, 12);
        indicators[i]->color(StateIndicator::State::OFF, base_theme::color::getMacroColor(i * 2))
                     .color(StateIndicator::State::ACTIVE, base_theme::color::getMacroColor(i * 2))
                     .opacity(StateIndicator::State::OFF, LV_OPA_40)
                     .opacity(StateIndicator::State::ACTIVE, LV_OPA_COVER);
        indicators[i]->setState(i < 2 ? StateIndicator::State::ACTIVE : StateIndicator::State::OFF);
    }

    status_label = new Label(ind_container);
    status_label->color(base_theme::color::TEXT_PRIMARY).alignment(LV_TEXT_ALIGN_CENTER);
    status_label->setText("Status");

    // Empty slot 7 (placeholder)
    lv_obj_t* placeholder = lv_obj_create(body);
    lv_obj_set_style_bg_opa(placeholder, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(placeholder, 0, 0);
    lv_obj_set_grid_cell(placeholder, LV_GRID_ALIGN_STRETCH, 3, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    // ========================================================================
    // TransportBar (20px height)
    // ========================================================================
    lv_obj_t* transport_bar = lv_obj_create(screen_container);
    lv_obj_set_size(transport_bar, LV_PCT(100), TRANSPORT_BAR_HEIGHT);
    lv_obj_set_style_bg_color(transport_bar, lv_color_hex(base_theme::color::INACTIVE), 0);
    lv_obj_set_style_border_width(transport_bar, 0, 0);
    lv_obj_set_style_pad_all(transport_bar, 0, 0);
    lv_obj_set_scrollbar_mode(transport_bar, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t* transport_label = lv_label_create(transport_bar);
    lv_label_set_text(transport_label, "120.0 BPM  |  1.1.1");
    lv_obj_set_style_text_color(transport_label, lv_color_hex(base_theme::color::TEXT_SECONDARY), 0);
    lv_obj_center(transport_label);
}
