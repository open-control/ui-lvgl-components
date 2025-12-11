#include <oc/ui/lvgl/widget/EnumWidget.hpp>

#include <utility>

namespace oc::ui::lvgl {

EnumWidget::EnumWidget(lv_obj_t* parent) {
    container_ = lv_obj_create(parent);
    createUI();
}

EnumWidget::~EnumWidget() {
    cleanup();
}

EnumWidget::EnumWidget(EnumWidget&& other) noexcept
    : container_(other.container_),
      inner_(other.inner_),
      top_line_(other.top_line_),
      flash_timer_(other.flash_timer_),
      bg_color_(other.bg_color_),
      line_color_(other.line_color_),
      flash_color_(other.flash_color_) {
    other.container_ = nullptr;
    other.inner_ = nullptr;
    other.top_line_ = nullptr;
    other.flash_timer_ = nullptr;
}

EnumWidget& EnumWidget::operator=(EnumWidget&& other) noexcept {
    if (this != &other) {
        cleanup();

        container_ = other.container_;
        inner_ = other.inner_;
        top_line_ = other.top_line_;
        flash_timer_ = other.flash_timer_;
        bg_color_ = other.bg_color_;
        line_color_ = other.line_color_;
        flash_color_ = other.flash_color_;

        other.container_ = nullptr;
        other.inner_ = nullptr;
        other.top_line_ = nullptr;
        other.flash_timer_ = nullptr;
    }
    return *this;
}

void EnumWidget::cleanup() {
    if (flash_timer_) {
        lv_timer_delete(flash_timer_);
        flash_timer_ = nullptr;
    }
    if (container_) {
        lv_obj_delete(container_);
        container_ = nullptr;
    }
    inner_ = nullptr;
    top_line_ = nullptr;
}

void EnumWidget::createUI() {
    // Container: 100% of parent (adapts to grid cell)
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(container_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(container_, 0, 0);
    lv_obj_set_style_pad_all(container_, 0, 0);
    lv_obj_set_scrollbar_mode(container_, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_layout(container_, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(container_, LV_OBJ_FLAG_EVENT_BUBBLE);

    // Indicator line (at top)
    top_line_ = lv_obj_create(container_);
    lv_obj_set_size(top_line_, lv_pct(100), LINE_HEIGHT);
    lv_obj_set_style_bg_opa(top_line_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(top_line_, 0, 0);
    lv_obj_set_style_radius(top_line_, 0, 0);
    lv_obj_set_style_margin_left(top_line_, LINE_MARGIN, 0);
    lv_obj_set_style_margin_right(top_line_, LINE_MARGIN, 0);
    lv_obj_set_style_margin_top(top_line_, LINE_TOP_MARGIN, 0);
    lv_obj_set_style_margin_bottom(top_line_, LINE_BOTTOM_MARGIN, 0);
    lv_obj_add_flag(top_line_, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_set_scrollbar_mode(top_line_, LV_SCROLLBAR_MODE_OFF);

    // Inner content area
    inner_ = lv_obj_create(container_);
    lv_obj_set_width(inner_, lv_pct(100));
    lv_obj_set_height(inner_, INNER_HEIGHT);
    lv_obj_set_style_bg_opa(inner_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(inner_, 0, 0);
    lv_obj_set_style_pad_all(inner_, 0, 0);
    lv_obj_set_style_margin_left(inner_, LINE_MARGIN, 0);
    lv_obj_set_style_margin_right(inner_, LINE_MARGIN, 0);
    lv_obj_set_scrollbar_mode(inner_, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(inner_, LV_OBJ_FLAG_EVENT_BUBBLE);

    applyColors();
}

void EnumWidget::applyColors() {
    uint32_t bg = bg_color_ != 0 ? bg_color_ : BaseTheme::Color::BACKGROUND;
    uint32_t line = line_color_ != 0 ? line_color_ : BaseTheme::Color::INACTIVE;

    if (container_) {
        lv_obj_set_style_bg_color(container_, lv_color_hex(bg), 0);
        lv_obj_set_style_bg_opa(container_, bg_color_ != 0 ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
    }
    if (top_line_) {
        lv_obj_set_style_bg_color(top_line_, lv_color_hex(line), 0);
    }
}

// Fluent setters
EnumWidget& EnumWidget::bgColor(uint32_t color) & {
    bg_color_ = color;
    applyColors();
    return *this;
}

EnumWidget EnumWidget::bgColor(uint32_t color) && {
    return std::move(bgColor(color));
}

EnumWidget& EnumWidget::lineColor(uint32_t color) & {
    line_color_ = color;
    applyColors();
    return *this;
}

EnumWidget EnumWidget::lineColor(uint32_t color) && {
    return std::move(lineColor(color));
}

EnumWidget& EnumWidget::flashColor(uint32_t color) & {
    flash_color_ = color;
    return *this;
}

EnumWidget EnumWidget::flashColor(uint32_t color) && {
    return std::move(flashColor(color));
}

void EnumWidget::triggerFlash() {
    if (!top_line_) return;

    if (flash_timer_) {
        lv_timer_delete(flash_timer_);
        flash_timer_ = nullptr;
    }

    uint32_t flash = flash_color_ != 0 ? flash_color_ : BaseTheme::Color::ACTIVE;
    lv_obj_set_style_bg_color(top_line_, lv_color_hex(flash), 0);

    flash_timer_ = lv_timer_create(flashTimerCallback, BaseTheme::Animation::FLASH_DURATION_MS, this);
    lv_timer_set_repeat_count(flash_timer_, 1);
}

void EnumWidget::flashTimerCallback(lv_timer_t* timer) {
    auto* widget = static_cast<EnumWidget*>(lv_timer_get_user_data(timer));
    if (!widget || !widget->top_line_) return;

    uint32_t line = widget->line_color_ != 0 ? widget->line_color_ : BaseTheme::Color::INACTIVE;
    lv_obj_set_style_bg_color(widget->top_line_, lv_color_hex(line), 0);
    widget->flash_timer_ = nullptr;
}

}  // namespace oc::ui::lvgl
