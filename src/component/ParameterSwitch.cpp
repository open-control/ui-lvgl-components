#include <oc/ui/lvgl/component/ParameterSwitch.hpp>

#include <oc/ui/lvgl/theme/BaseTheme.hpp>

namespace oc::ui::lvgl {

ParameterSwitch::ParameterSwitch(lv_obj_t* parent) {
    createUI(parent);
}

ParameterSwitch::~ParameterSwitch() {
    cleanup();
}

ParameterSwitch::ParameterSwitch(ParameterSwitch&& other) noexcept
    : container_(other.container_),
      button_(std::move(other.button_)),
      label_(std::move(other.label_)) {
    other.container_ = nullptr;
}

ParameterSwitch& ParameterSwitch::operator=(ParameterSwitch&& other) noexcept {
    if (this != &other) {
        cleanup();
        container_ = other.container_;
        button_ = std::move(other.button_);
        label_ = std::move(other.label_);
        other.container_ = nullptr;
    }
    return *this;
}

void ParameterSwitch::show() {
    if (container_) lv_obj_clear_flag(container_, LV_OBJ_FLAG_HIDDEN);
}

void ParameterSwitch::hide() {
    if (container_) lv_obj_add_flag(container_, LV_OBJ_FLAG_HIDDEN);
}

bool ParameterSwitch::isVisible() const {
    return container_ && !lv_obj_has_flag(container_, LV_OBJ_FLAG_HIDDEN);
}

void ParameterSwitch::createUI(lv_obj_t* parent) {
    // Container fills parent, uses flex column layout
    container_ = lv_obj_create(parent);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(container_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(container_, 0, 0);
    lv_obj_set_style_pad_all(container_, 0, 0);
    lv_obj_set_scrollbar_mode(container_, LV_SCROLLBAR_MODE_OFF);

    // Flex column: widget takes remaining space (centered), label has fixed height
    lv_obj_set_layout(container_, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Button - centered, takes remaining space (flex_grow)
    button_ = std::make_unique<ButtonWidget>(container_);
    lv_obj_set_flex_grow(button_->getElement(), 1);

    // Label - fixed height, full width
    static constexpr int32_t LABEL_HEIGHT = 18;
    label_ = std::make_unique<Label>(container_);
    lv_obj_set_size(label_->getElement(), lv_pct(100), LABEL_HEIGHT);
    label_->alignment(LV_TEXT_ALIGN_CENTER)
           .color(BaseTheme::Color::TEXT_PRIMARY)
           .autoScroll(true);
}

void ParameterSwitch::cleanup() {
    button_.reset();
    label_.reset();
    if (container_) {
        lv_obj_delete(container_);
        container_ = nullptr;
    }
}

}  // namespace oc::ui::lvgl
