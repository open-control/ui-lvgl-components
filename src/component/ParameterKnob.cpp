#include <oc/ui/lvgl/component/ParameterKnob.hpp>

#include <oc/ui/lvgl/theme/BaseTheme.hpp>

namespace oc::ui::lvgl {

ParameterKnob::ParameterKnob(lv_obj_t* parent) {
    createUI(parent);
}

ParameterKnob::~ParameterKnob() {
    cleanup();
}

ParameterKnob::ParameterKnob(ParameterKnob&& other) noexcept
    : container_(other.container_),
      knob_(std::move(other.knob_)),
      label_(std::move(other.label_)) {
    other.container_ = nullptr;
}

ParameterKnob& ParameterKnob::operator=(ParameterKnob&& other) noexcept {
    if (this != &other) {
        cleanup();
        container_ = other.container_;
        knob_ = std::move(other.knob_);
        label_ = std::move(other.label_);
        other.container_ = nullptr;
    }
    return *this;
}

void ParameterKnob::show() {
    if (container_) lv_obj_clear_flag(container_, LV_OBJ_FLAG_HIDDEN);
}

void ParameterKnob::hide() {
    if (container_) lv_obj_add_flag(container_, LV_OBJ_FLAG_HIDDEN);
}

bool ParameterKnob::isVisible() const {
    return container_ && !lv_obj_has_flag(container_, LV_OBJ_FLAG_HIDDEN);
}

void ParameterKnob::createUI(lv_obj_t* parent) {
    // Container - 100% of parent, flex column layout
    container_ = lv_obj_create(parent);
    lv_obj_set_size(container_, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(container_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(container_, 0, 0);
    lv_obj_set_style_pad_all(container_, 0, 0);
    lv_obj_add_flag(container_, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    lv_obj_set_scrollbar_mode(container_, LV_SCROLLBAR_MODE_OFF);

    // Flex column: knob grows, label stays content height
    lv_obj_set_layout(container_, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // KnobWidget - takes remaining space (flex grow)
    knob_ = std::make_unique<KnobWidget>(container_);
    lv_obj_set_width(knob_->getElement(), LV_PCT(100));
    lv_obj_set_flex_grow(knob_->getElement(), 1);

    // Label - content height, full width
    label_ = std::make_unique<Label>(container_);
    lv_obj_set_width(label_->getElement(), LV_PCT(100));
    label_->alignment(LV_TEXT_ALIGN_CENTER)
           .color(BaseTheme::Color::TEXT_PRIMARY)
           .autoScroll(true);
}

void ParameterKnob::cleanup() {
    knob_.reset();
    label_.reset();
    if (container_) {
        lv_obj_delete(container_);
        container_ = nullptr;
    }
}

}  // namespace oc::ui::lvgl
