#pragma once

#include <cstdint>

#include <lvgl.h>

#include <oc/ui/lvgl/IWidget.hpp>

#include "../theme/BaseTheme.hpp"

namespace oc::ui::lvgl {

/**
 * @brief Toggle button widget with ON/OFF states
 *
 * Provides a button with customizable colors for each state.
 * Content can be added to inner() or use setText() for simple text.
 *
 * Usage:
 * @code
 * ButtonWidget button(parent);
 * button.offColor(0x333333)
 *       .onColor(0xECA747)
 *       .textOffColor(0xFFFFFF)
 *       .textOnColor(0x292929);
 * button.setText("OFF");
 * button.setState(true);  // Changes to ON state
 * @endcode
 */
class ButtonWidget : public IWidget {
public:
    explicit ButtonWidget(lv_obj_t* parent);
    ~ButtonWidget();

    // Move only
    ButtonWidget(ButtonWidget&& other) noexcept;
    ButtonWidget& operator=(ButtonWidget&& other) noexcept;
    ButtonWidget(const ButtonWidget&) = delete;
    ButtonWidget& operator=(const ButtonWidget&) = delete;

    // LVGL Access
    lv_obj_t* getElement() const override { return container_; }
    operator lv_obj_t*() const { return container_; }

    /** @brief Parent object for adding custom content */
    lv_obj_t* inner() const { return button_box_; }

    // Fluent Configuration
    ButtonWidget& offColor(uint32_t color);      ///< Background when OFF
    ButtonWidget& onColor(uint32_t color);       ///< Background when ON
    ButtonWidget& textOffColor(uint32_t color);  ///< Text color when OFF
    ButtonWidget& textOnColor(uint32_t color);   ///< Text color when ON

    // State
    void setState(bool on);
    bool getState() const { return is_on_; }

    /** @brief Set text (creates internal label if needed) */
    void setText(const char* text);

private:
    // Sizing ratios (relative to min(parent_w, parent_h))
    static constexpr float BUTTON_SIZE_RATIO = 0.6f;  // 60% of min parent dimension
    static constexpr float RADIUS_RATIO = 0.15f;      // Corner radius relative to button size
    static constexpr lv_coord_t MIN_SIZE = 20;        // Minimum button size

    void createUI();
    void cleanup();
    void applyState();
    void updateGeometry();
    static void sizeChangedCallback(lv_event_t* e);

    // LVGL objects
    lv_obj_t* container_ = nullptr;
    lv_obj_t* button_box_ = nullptr;
    lv_obj_t* state_label_ = nullptr;  // Created by setText(), nullptr otherwise

    // Cached size
    lv_coord_t button_size_ = 0;

    // State
    bool is_on_ = false;

    // Configuration
    uint32_t off_color_ = 0;
    uint32_t on_color_ = 0;
    uint32_t text_off_color_ = 0;
    uint32_t text_on_color_ = 0;
};

}  // namespace oc::ui::lvgl
