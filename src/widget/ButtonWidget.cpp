#include <oc/ui/lvgl/widget/ButtonWidget.hpp>

#include <algorithm>
#include <utility>

namespace oc::ui::lvgl {

ButtonWidget::ButtonWidget(lv_obj_t* parent) {
    container_ = lv_obj_create(parent);
    createUI();
}

ButtonWidget::~ButtonWidget() {
    cleanup();
}

ButtonWidget::ButtonWidget(ButtonWidget&& other) noexcept
    : container_(other.container_),
      button_box_(other.button_box_),
      state_label_(other.state_label_),
      button_size_(other.button_size_),
      is_on_(other.is_on_),
      off_color_(other.off_color_),
      on_color_(other.on_color_),
      text_off_color_(other.text_off_color_),
      text_on_color_(other.text_on_color_) {
    other.container_ = nullptr;
    other.button_box_ = nullptr;
    other.state_label_ = nullptr;
}

ButtonWidget& ButtonWidget::operator=(ButtonWidget&& other) noexcept {
    if (this != &other) {
        cleanup();

        container_ = other.container_;
        button_box_ = other.button_box_;
        state_label_ = other.state_label_;
        button_size_ = other.button_size_;
        is_on_ = other.is_on_;
        off_color_ = other.off_color_;
        on_color_ = other.on_color_;
        text_off_color_ = other.text_off_color_;
        text_on_color_ = other.text_on_color_;

        other.container_ = nullptr;
        other.button_box_ = nullptr;
        other.state_label_ = nullptr;
    }
    return *this;
}

void ButtonWidget::cleanup() {
    if (container_) {
        lv_obj_delete(container_);
        container_ = nullptr;
    }
    button_box_ = nullptr;
    state_label_ = nullptr;
}

void ButtonWidget::createUI() {
    // Container setup - transparent, no padding
    lv_obj_set_style_bg_opa(container_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(container_, 0, 0);
    lv_obj_set_style_pad_all(container_, 0, 0);
    lv_obj_set_style_pad_row(container_, 0, 0);
    lv_obj_set_style_pad_column(container_, 0, 0);
    lv_obj_set_scrollbar_mode(container_, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(container_, LV_OBJ_FLAG_EVENT_BUBBLE);

    // Button box (centered, rounded)
    button_box_ = lv_obj_create(container_);
    lv_obj_center(button_box_);
    lv_obj_set_style_border_width(button_box_, 0, 0);
    lv_obj_set_style_bg_opa(button_box_, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(button_box_, 0, 0);
    lv_obj_set_scrollbar_mode(button_box_, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(button_box_, LV_OBJ_FLAG_EVENT_BUBBLE);

    applyState();

    // Listen for parent size changes
    lv_obj_t* parent = lv_obj_get_parent(container_);
    if (parent) {
        lv_obj_add_event_cb(parent, sizeChangedCallback, LV_EVENT_SIZE_CHANGED, this);
    }

    // Defer initial geometry calculation
    lv_timer_t* init_timer = lv_timer_create([](lv_timer_t* t) {
        auto* widget = static_cast<ButtonWidget*>(lv_timer_get_user_data(t));
        if (widget) widget->updateGeometry();
    }, 0, this);
    lv_timer_set_repeat_count(init_timer, 1);
}

void ButtonWidget::sizeChangedCallback(lv_event_t* e) {
    auto* widget = static_cast<ButtonWidget*>(lv_event_get_user_data(e));
    if (widget) {
        widget->updateGeometry();
    }
}

void ButtonWidget::updateGeometry() {
    lv_obj_t* parent = lv_obj_get_parent(container_);
    if (!parent) return;

    // Force parent layout update
    lv_obj_update_layout(parent);

    // Get available space (account for sibling label if in ParameterButton)
    lv_coord_t parent_w = lv_obj_get_width(parent);
    lv_coord_t parent_h = lv_obj_get_height(parent);

    // Find sibling label height (if in a component with label)
    lv_coord_t label_h = 0;
    uint32_t child_count = lv_obj_get_child_count(parent);
    for (uint32_t i = 0; i < child_count; i++) {
        lv_obj_t* child = lv_obj_get_child(parent, i);
        if (child != container_) {
            label_h = lv_obj_get_height(child);
            break;
        }
    }

    // Available space for button
    lv_coord_t available_h = parent_h - label_h;
    lv_coord_t min_dim = std::min(parent_w, available_h);

    // Button size = 60% of min dimension, with minimum
    button_size_ = std::max(MIN_SIZE, static_cast<lv_coord_t>(min_dim * BUTTON_SIZE_RATIO));

    // Set container to button size (centered by grid)
    lv_obj_set_size(container_, button_size_, button_size_);

    // Button box fills container
    lv_obj_set_size(button_box_, button_size_, button_size_);
    lv_obj_center(button_box_);

    // Radius proportional to size
    lv_coord_t radius = static_cast<lv_coord_t>(button_size_ * RADIUS_RATIO);
    lv_obj_set_style_radius(button_box_, radius, 0);
}

void ButtonWidget::applyState() {
    uint32_t bg_default_off = BaseTheme::Color::INACTIVE;
    uint32_t bg_default_on = BaseTheme::Color::ACTIVE;
    uint32_t text_default_off = BaseTheme::Color::TEXT_PRIMARY;
    uint32_t text_default_on = BaseTheme::Color::TEXT_PRIMARY_INVERTED;

    uint32_t bg = is_on_ ? (on_color_ != 0 ? on_color_ : bg_default_on)
                         : (off_color_ != 0 ? off_color_ : bg_default_off);

    uint32_t text = is_on_ ? (text_on_color_ != 0 ? text_on_color_ : text_default_on)
                           : (text_off_color_ != 0 ? text_off_color_ : text_default_off);

    if (button_box_) {
        lv_obj_set_style_bg_color(button_box_, lv_color_hex(bg), 0);
    }
    if (state_label_) {
        lv_obj_set_style_text_color(state_label_, lv_color_hex(text), 0);
    }
}

// Fluent setters
ButtonWidget& ButtonWidget::offColor(uint32_t color) {
    off_color_ = color;
    applyState();
    return *this;
}

ButtonWidget& ButtonWidget::onColor(uint32_t color) {
    on_color_ = color;
    applyState();
    return *this;
}

ButtonWidget& ButtonWidget::textOffColor(uint32_t color) {
    text_off_color_ = color;
    applyState();
    return *this;
}

ButtonWidget& ButtonWidget::textOnColor(uint32_t color) {
    text_on_color_ = color;
    applyState();
    return *this;
}

void ButtonWidget::setState(bool on) {
    if (is_on_ == on) return;
    is_on_ = on;
    applyState();
}

void ButtonWidget::setText(const char* text) {
    if (!state_label_) {
        state_label_ = lv_label_create(button_box_);
        lv_obj_center(state_label_);
        lv_obj_set_style_text_align(state_label_, LV_TEXT_ALIGN_CENTER, 0);
    }
    lv_label_set_text(state_label_, text);
    applyState();  // Apply text color
}

}  // namespace oc::ui::lvgl
