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
    // Container - 100% of parent, grid layout (same pattern as ParameterKnob)
    container_ = lv_obj_create(parent);
    lv_obj_set_size(container_, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(container_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(container_, 0, 0);
    lv_obj_set_style_pad_all(container_, 0, 0);
    lv_obj_set_style_pad_row(container_, 0, 0);
    lv_obj_set_style_pad_column(container_, 0, 0);
    lv_obj_set_scrollbar_mode(container_, LV_SCROLLBAR_MODE_OFF);

    // Grid: 1 column (100%), 2 rows (FR(1) for button, CONTENT for label)
    static int32_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static int32_t row_dsc[] = {LV_GRID_FR(1), LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(container_, col_dsc, row_dsc);
    lv_obj_set_layout(container_, LV_LAYOUT_GRID);

    // Row 0: ButtonWidget - centered in remaining space
    button_ = std::make_unique<ButtonWidget>(container_);
    lv_obj_set_grid_cell(button_->getElement(),
        LV_GRID_ALIGN_CENTER, 0, 1,   // col: center
        LV_GRID_ALIGN_CENTER, 0, 1);  // row: center in row 0

    // Row 1: Label - stretch width, content height
    label_ = std::make_unique<Label>(container_);
    lv_obj_set_grid_cell(label_->getElement(),
        LV_GRID_ALIGN_STRETCH, 0, 1,  // col: stretch full width
        LV_GRID_ALIGN_CENTER, 1, 1);  // row: center in row 1
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
