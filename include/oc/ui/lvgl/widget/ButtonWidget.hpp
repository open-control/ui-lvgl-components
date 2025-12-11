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
    /** @brief Background color when OFF */
    ButtonWidget& offColor(uint32_t color) &;
    ButtonWidget offColor(uint32_t color) &&;

    /** @brief Background color when ON */
    ButtonWidget& onColor(uint32_t color) &;
    ButtonWidget onColor(uint32_t color) &&;

    /** @brief Text color when OFF */
    ButtonWidget& textOffColor(uint32_t color) &;
    ButtonWidget textOffColor(uint32_t color) &&;

    /** @brief Text color when ON */
    ButtonWidget& textOnColor(uint32_t color) &;
    ButtonWidget textOnColor(uint32_t color) &&;

    // State
    void setState(bool on);
    bool getState() const { return is_on_; }

    /** @brief Set text (creates internal label if needed) */
    void setText(const char* text);

private:
    static constexpr uint16_t DEFAULT_SIZE = 62;
    static constexpr uint16_t BUTTON_SIZE = 40;
    static constexpr uint8_t BUTTON_RADIUS = 8;

    void createUI();
    void cleanup();
    void applyState();

    // LVGL objects
    lv_obj_t* container_ = nullptr;
    lv_obj_t* button_box_ = nullptr;
    lv_obj_t* state_label_ = nullptr;  // Created by setText(), nullptr otherwise

    // State
    bool is_on_ = false;

    // Configuration
    uint32_t off_color_ = 0;
    uint32_t on_color_ = 0;
    uint32_t text_off_color_ = 0;
    uint32_t text_on_color_ = 0;
};

}  // namespace oc::ui::lvgl
