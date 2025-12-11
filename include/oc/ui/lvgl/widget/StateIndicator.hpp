#pragma once

#include <cstdint>

#include <lvgl.h>

#include <oc/ui/lvgl/IWidget.hpp>

#include "../theme/BaseTheme.hpp"

namespace oc::ui::lvgl {

/**
 * @brief Circular state indicator (LED-style)
 *
 * Simple circular indicator with customizable colors and opacities
 * for different states: OFF, ACTIVE, PRESSED.
 *
 * Usage:
 * @code
 * StateIndicator indicator(parent, 12);
 * indicator.color(StateIndicator::State::OFF, 0x606060)
 *          .color(StateIndicator::State::ACTIVE, 0x00FF00)
 *          .opacity(StateIndicator::State::OFF, LV_OPA_60);
 * indicator.setState(StateIndicator::State::ACTIVE);
 * @endcode
 */
class StateIndicator : public IWidget {
public:
    enum class State { OFF = 0, ACTIVE = 1, PRESSED = 2 };

    explicit StateIndicator(lv_obj_t* parent, lv_coord_t size = 12);
    ~StateIndicator();

    // Move only
    StateIndicator(StateIndicator&& other) noexcept;
    StateIndicator& operator=(StateIndicator&& other) noexcept;
    StateIndicator(const StateIndicator&) = delete;
    StateIndicator& operator=(const StateIndicator&) = delete;

    // LVGL Access
    lv_obj_t* getElement() const override { return led_; }
    operator lv_obj_t*() const { return led_; }

    // Fluent Configuration
    /** @brief Set color for a specific state */
    StateIndicator& color(State state, uint32_t color) &;
    StateIndicator color(State state, uint32_t color) &&;

    /** @brief Set opacity for a specific state */
    StateIndicator& opacity(State state, lv_opa_t opacity) &;
    StateIndicator opacity(State state, lv_opa_t opacity) &&;

    // State
    void setState(State state);
    State getState() const { return current_state_; }

private:
    void applyState();

    lv_obj_t* led_ = nullptr;
    State current_state_ = State::OFF;

    // Colors indexed by State enum (0=OFF, 1=ACTIVE, 2=PRESSED)
    uint32_t colors_[3] = {0, 0, 0};
    // Opacities indexed by State enum, 0 = use default
    lv_opa_t opacities_[3] = {0, 0, 0};
};

}  // namespace oc::ui::lvgl
