#include <oc/ui/lvgl/widget/ButtonWidget.hpp>

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
    // Container setup
    lv_obj_set_size(container_, DEFAULT_SIZE, DEFAULT_SIZE);
    lv_obj_set_style_bg_opa(container_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(container_, 0, 0);
    lv_obj_set_style_pad_all(container_, 0, 0);
    lv_obj_set_scrollbar_mode(container_, LV_SCROLLBAR_MODE_OFF);

    // Button box (centered, rounded)
    button_box_ = lv_obj_create(container_);
    lv_obj_set_size(button_box_, BUTTON_SIZE, BUTTON_SIZE);
    lv_obj_center(button_box_);
    lv_obj_set_style_radius(button_box_, BUTTON_RADIUS, 0);
    lv_obj_set_style_border_width(button_box_, 0, 0);
    lv_obj_set_style_bg_opa(button_box_, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(button_box_, 0, 0);
    lv_obj_set_scrollbar_mode(button_box_, LV_SCROLLBAR_MODE_OFF);

    applyState();
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
ButtonWidget& ButtonWidget::offColor(uint32_t color) & {
    off_color_ = color;
    applyState();
    return *this;
}

ButtonWidget ButtonWidget::offColor(uint32_t color) && {
    return std::move(offColor(color));
}

ButtonWidget& ButtonWidget::onColor(uint32_t color) & {
    on_color_ = color;
    applyState();
    return *this;
}

ButtonWidget ButtonWidget::onColor(uint32_t color) && {
    return std::move(onColor(color));
}

ButtonWidget& ButtonWidget::textOffColor(uint32_t color) & {
    text_off_color_ = color;
    applyState();
    return *this;
}

ButtonWidget ButtonWidget::textOffColor(uint32_t color) && {
    return std::move(textOffColor(color));
}

ButtonWidget& ButtonWidget::textOnColor(uint32_t color) & {
    text_on_color_ = color;
    applyState();
    return *this;
}

ButtonWidget ButtonWidget::textOnColor(uint32_t color) && {
    return std::move(textOnColor(color));
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
