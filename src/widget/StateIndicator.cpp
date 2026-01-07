#include <oc/ui/lvgl/widget/StateIndicator.hpp>

#include <utility>

namespace oc::ui::lvgl {

StateIndicator::StateIndicator(lv_obj_t* parent, lv_coord_t size) {
    led_ = lv_obj_create(parent);
    lv_obj_set_size(led_, size, size);
    lv_obj_set_style_radius(led_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(led_, 0, 0);
    lv_obj_set_style_bg_opa(led_, LV_OPA_COVER, 0);
    lv_obj_set_scrollbar_mode(led_, LV_SCROLLBAR_MODE_OFF);

    applyState();
}

StateIndicator::~StateIndicator() {
    if (led_) {
        lv_obj_delete(led_);
        led_ = nullptr;
    }
}

StateIndicator::StateIndicator(StateIndicator&& other) noexcept
    : led_(other.led_),
      current_state_(other.current_state_) {
    for (int i = 0; i < 3; ++i) {
        colors_[i] = other.colors_[i];
        opacities_[i] = other.opacities_[i];
    }
    other.led_ = nullptr;
}

StateIndicator& StateIndicator::operator=(StateIndicator&& other) noexcept {
    if (this != &other) {
        if (led_) {
            lv_obj_delete(led_);
        }

        led_ = other.led_;
        current_state_ = other.current_state_;
        for (int i = 0; i < 3; ++i) {
            colors_[i] = other.colors_[i];
            opacities_[i] = other.opacities_[i];
        }

        other.led_ = nullptr;
    }
    return *this;
}

void StateIndicator::applyState() {
    if (!led_) return;

    int idx = static_cast<int>(current_state_);

    // Default colors per state
    static constexpr uint32_t DEFAULT_COLORS[] = {
        base_theme::color::INACTIVE,        // OFF
        base_theme::color::STATUS_WARNING,  // ACTIVE
        base_theme::color::STATUS_SUCCESS   // PRESSED
    };

    // Default opacities per state
    static constexpr lv_opa_t DEFAULT_OPACITIES[] = {
        LV_OPA_60,    // OFF
        LV_OPA_80,    // ACTIVE
        LV_OPA_COVER  // PRESSED
    };

    uint32_t col = colors_[idx] != 0 ? colors_[idx] : DEFAULT_COLORS[idx];
    lv_opa_t opa = opacities_[idx] != 0 ? opacities_[idx] : DEFAULT_OPACITIES[idx];

    lv_obj_set_style_bg_color(led_, lv_color_hex(col), 0);
    lv_obj_set_style_bg_opa(led_, opa, 0);
}

// Fluent setters
StateIndicator& StateIndicator::color(State state, uint32_t c) & {
    colors_[static_cast<int>(state)] = c;
    if (current_state_ == state) {
        applyState();
    }
    return *this;
}

StateIndicator StateIndicator::color(State state, uint32_t c) && {
    return std::move(color(state, c));
}

StateIndicator& StateIndicator::opacity(State state, lv_opa_t opa) & {
    opacities_[static_cast<int>(state)] = opa;
    if (current_state_ == state) {
        applyState();
    }
    return *this;
}

StateIndicator StateIndicator::opacity(State state, lv_opa_t opa) && {
    return std::move(opacity(state, opa));
}

void StateIndicator::setState(State state) {
    if (current_state_ == state) return;
    current_state_ = state;
    applyState();
}

}  // namespace oc::ui::lvgl
