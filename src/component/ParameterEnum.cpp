#include <oc/ui/lvgl/component/ParameterEnum.hpp>

#include <oc/ui/lvgl/theme/BaseTheme.hpp>

namespace oc::ui::lvgl {

ParameterEnum::ParameterEnum(lv_obj_t* parent) {
    createUI(parent);
}

ParameterEnum::~ParameterEnum() {
    cleanup();
}

ParameterEnum::ParameterEnum(ParameterEnum&& other) noexcept
    : container_(other.container_),
      enum_widget_(std::move(other.enum_widget_)),
      value_label_(std::move(other.value_label_)),
      name_label_(std::move(other.name_label_)) {
    other.container_ = nullptr;
}

ParameterEnum& ParameterEnum::operator=(ParameterEnum&& other) noexcept {
    if (this != &other) {
        cleanup();
        container_ = other.container_;
        enum_widget_ = std::move(other.enum_widget_);
        value_label_ = std::move(other.value_label_);
        name_label_ = std::move(other.name_label_);
        other.container_ = nullptr;
    }
    return *this;
}

void ParameterEnum::show() {
    if (container_) lv_obj_clear_flag(container_, LV_OBJ_FLAG_HIDDEN);
}

void ParameterEnum::hide() {
    if (container_) lv_obj_add_flag(container_, LV_OBJ_FLAG_HIDDEN);
}

bool ParameterEnum::isVisible() const {
    return container_ && !lv_obj_has_flag(container_, LV_OBJ_FLAG_HIDDEN);
}

void ParameterEnum::createUI(lv_obj_t* parent) {
    // Container fills parent, uses flex column layout
    container_ = lv_obj_create(parent);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(container_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(container_, 0, 0);
    lv_obj_set_style_pad_all(container_, 0, 0);
    lv_obj_set_scrollbar_mode(container_, LV_SCROLLBAR_MODE_OFF);

    // Flex column: widget takes remaining space, label has fixed height
    lv_obj_set_layout(container_, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Enum widget - takes all available space (flex_grow)
    enum_widget_ = std::make_unique<EnumWidget>(container_);
    lv_obj_set_width(enum_widget_->getElement(), lv_pct(100));
    lv_obj_set_flex_grow(enum_widget_->getElement(), 1);

    // Value label (inside enum widget inner area - which is flex)
    value_label_ = std::make_unique<Label>(enum_widget_->inner());
    lv_obj_set_size(value_label_->getElement(), lv_pct(100), LV_SIZE_CONTENT);
    value_label_->alignment(LV_TEXT_ALIGN_CENTER)
                 .color(BaseTheme::Color::TEXT_PRIMARY)
                 .autoScroll(true);

    // Name label - fixed height, full width
    static constexpr int32_t LABEL_HEIGHT = 18;
    name_label_ = std::make_unique<Label>(container_);
    lv_obj_set_size(name_label_->getElement(), lv_pct(100), LABEL_HEIGHT);
    name_label_->alignment(LV_TEXT_ALIGN_CENTER)
               .color(BaseTheme::Color::TEXT_PRIMARY)
               .autoScroll(true);
}

void ParameterEnum::cleanup() {
    value_label_.reset();
    enum_widget_.reset();
    name_label_.reset();
    if (container_) {
        lv_obj_delete(container_);
        container_ = nullptr;
    }
}

}  // namespace oc::ui::lvgl
