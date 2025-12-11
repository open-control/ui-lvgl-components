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
    // Container - 100% of parent, grid layout (same pattern as other Parameter* components)
    container_ = lv_obj_create(parent);
    lv_obj_set_size(container_, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(container_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(container_, 0, 0);
    lv_obj_set_style_pad_all(container_, 0, 0);
    lv_obj_set_style_pad_row(container_, 0, 0);
    lv_obj_set_style_pad_column(container_, 0, 0);
    lv_obj_set_scrollbar_mode(container_, LV_SCROLLBAR_MODE_OFF);

    // Grid: 1 column (100%), 2 rows (FR(1) for enum widget, CONTENT for label)
    static int32_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static int32_t row_dsc[] = {LV_GRID_FR(1), LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(container_, col_dsc, row_dsc);
    lv_obj_set_layout(container_, LV_LAYOUT_GRID);

    // Row 0: EnumWidget - stretch to fill remaining space
    enum_widget_ = std::make_unique<EnumWidget>(container_);
    lv_obj_set_grid_cell(enum_widget_->getElement(),
        LV_GRID_ALIGN_STRETCH, 0, 1,  // col: stretch full width
        LV_GRID_ALIGN_STRETCH, 0, 1); // row: stretch in row 0

    // Value label (inside enum widget inner area)
    value_label_ = std::make_unique<Label>(enum_widget_->inner());
    lv_obj_set_size(value_label_->getElement(), LV_PCT(100), LV_SIZE_CONTENT);
    value_label_->alignment(LV_TEXT_ALIGN_CENTER)
                 .color(BaseTheme::Color::TEXT_PRIMARY)
                 .autoScroll(true);

    // Row 1: Name label - stretch width, content height
    name_label_ = std::make_unique<Label>(container_);
    lv_obj_set_grid_cell(name_label_->getElement(),
        LV_GRID_ALIGN_STRETCH, 0, 1,  // col: stretch full width
        LV_GRID_ALIGN_CENTER, 1, 1);  // row: center in row 1
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
