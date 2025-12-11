#include <oc/ui/lvgl/widget/KnobWidget.hpp>

#include <algorithm>
#include <cmath>

namespace oc::ui::lvgl {

KnobWidget::KnobWidget(lv_obj_t* parent) {
    container_ = lv_obj_create(parent);
    createUI();
}

KnobWidget::~KnobWidget() {
    cleanup();
}

KnobWidget::KnobWidget(KnobWidget&& other) noexcept
    : container_(other.container_),
      arc_(other.arc_),
      indicator_(other.indicator_),
      center_circle_(other.center_circle_),
      inner_circle_(other.inner_circle_),
      flash_timer_(other.flash_timer_),
      bg_color_(other.bg_color_),
      track_color_(other.track_color_),
      value_color_(other.value_color_),
      flash_color_(other.flash_color_),
      value_(other.value_),
      origin_(other.origin_),
      centered_(other.centered_),
      knob_size_(other.knob_size_),
      arc_center_x_(other.arc_center_x_),
      arc_center_y_(other.arc_center_y_),
      arc_radius_(other.arc_radius_) {
    line_points_[0] = other.line_points_[0];
    line_points_[1] = other.line_points_[1];

    other.container_ = nullptr;
    other.arc_ = nullptr;
    other.indicator_ = nullptr;
    other.center_circle_ = nullptr;
    other.inner_circle_ = nullptr;
    other.flash_timer_ = nullptr;
}

KnobWidget& KnobWidget::operator=(KnobWidget&& other) noexcept {
    if (this != &other) {
        cleanup();

        container_ = other.container_;
        arc_ = other.arc_;
        indicator_ = other.indicator_;
        center_circle_ = other.center_circle_;
        inner_circle_ = other.inner_circle_;
        flash_timer_ = other.flash_timer_;
        line_points_[0] = other.line_points_[0];
        line_points_[1] = other.line_points_[1];
        bg_color_ = other.bg_color_;
        track_color_ = other.track_color_;
        value_color_ = other.value_color_;
        flash_color_ = other.flash_color_;
        value_ = other.value_;
        origin_ = other.origin_;
        centered_ = other.centered_;
        knob_size_ = other.knob_size_;
        arc_center_x_ = other.arc_center_x_;
        arc_center_y_ = other.arc_center_y_;
        arc_radius_ = other.arc_radius_;

        other.container_ = nullptr;
        other.arc_ = nullptr;
        other.indicator_ = nullptr;
        other.center_circle_ = nullptr;
        other.inner_circle_ = nullptr;
        other.flash_timer_ = nullptr;
    }
    return *this;
}

void KnobWidget::cleanup() {
    if (flash_timer_) {
        lv_timer_delete(flash_timer_);
        flash_timer_ = nullptr;
    }
    if (container_) {
        lv_obj_delete(container_);
        container_ = nullptr;
    }
    arc_ = nullptr;
    indicator_ = nullptr;
    center_circle_ = nullptr;
    inner_circle_ = nullptr;
}

void KnobWidget::createUI() {
    // Size will be controlled by parent (flex/grid)
    lv_obj_set_style_bg_opa(container_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(container_, 0, 0);
    lv_obj_set_style_pad_all(container_, 0, 0);
    lv_obj_add_flag(container_, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    lv_obj_set_scrollbar_mode(container_, LV_SCROLLBAR_MODE_OFF);

    createArc();
    createIndicator();
    createCenterCircles();
    applyColors();

    // Listen for size changes to recalculate geometry
    lv_obj_add_event_cb(container_, sizeChangedCallback, LV_EVENT_SIZE_CHANGED, this);

    // Defer initial geometry calculation to next frame when layout is ready
    lv_timer_t* init_timer = lv_timer_create([](lv_timer_t* t) {
        auto* widget = static_cast<KnobWidget*>(lv_timer_get_user_data(t));
        if (widget) widget->updateGeometry();
    }, 0, this);
    lv_timer_set_repeat_count(init_timer, 1);
}

void KnobWidget::createArc() {
    arc_ = lv_arc_create(container_);
    lv_obj_center(arc_);
    lv_obj_remove_flag(arc_, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(arc_, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_arc_set_bg_angles(arc_, START_ANGLE, END_ANGLE);
    lv_obj_remove_style(arc_, nullptr, LV_PART_KNOB);
}

void KnobWidget::createIndicator() {
    indicator_ = lv_line_create(container_);
    lv_obj_add_flag(indicator_, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_set_style_line_rounded(indicator_, true, 0);

    // Initialize line points at origin
    line_points_[0] = {0, 0};
    line_points_[1] = {0, 0};
    lv_line_set_points_mutable(indicator_, line_points_, 2);
}

void KnobWidget::createCenterCircles() {
    // Outer circle (value color)
    center_circle_ = lv_obj_create(container_);
    lv_obj_center(center_circle_);
    lv_obj_set_style_radius(center_circle_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(center_circle_, 0, 0);
    lv_obj_set_style_bg_opa(center_circle_, LV_OPA_COVER, 0);
    lv_obj_remove_flag(center_circle_, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(center_circle_, LV_OBJ_FLAG_EVENT_BUBBLE);

    // Inner circle (flashes on value change)
    inner_circle_ = lv_obj_create(container_);
    lv_obj_center(inner_circle_);
    lv_obj_set_style_radius(inner_circle_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(inner_circle_, 0, 0);
    lv_obj_set_style_bg_color(inner_circle_, lv_color_hex(BaseTheme::Color::INACTIVE), 0);
    lv_obj_set_style_bg_opa(inner_circle_, LV_OPA_COVER, 0);
    lv_obj_remove_flag(inner_circle_, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(inner_circle_, LV_OBJ_FLAG_EVENT_BUBBLE);
}

void KnobWidget::sizeChangedCallback(lv_event_t* e) {
    auto* widget = static_cast<KnobWidget*>(lv_event_get_user_data(e));
    if (widget) {
        widget->updateGeometry();
    }
}

void KnobWidget::updateGeometry() {
    if (!container_) return;

    lv_obj_update_layout(container_);
    lv_coord_t w = lv_obj_get_width(container_);
    lv_coord_t h = lv_obj_get_height(container_);

    // Skip if size not yet determined
    if (w <= 0 || h <= 0) return;

    // Use min(width, height) for square knob, enforce minimum
    knob_size_ = std::max(static_cast<lv_coord_t>(MIN_SIZE), std::min(w, h));

    // Center point of container
    arc_center_x_ = w / 2;
    arc_center_y_ = h / 2;

    // Compute sizes based on knob_size_
    lv_coord_t arc_width = static_cast<lv_coord_t>(knob_size_ * ARC_WIDTH_RATIO);
    lv_coord_t indicator_thickness = static_cast<lv_coord_t>(knob_size_ * INDICATOR_RATIO);
    lv_coord_t center_circle_size = static_cast<lv_coord_t>(knob_size_ * CENTER_CIRCLE_RATIO);
    lv_coord_t inner_circle_size = static_cast<lv_coord_t>(knob_size_ * INNER_CIRCLE_RATIO);

    // Arc size and radius (subtract indicator thickness for clean edges)
    lv_coord_t arc_size = knob_size_ - indicator_thickness;
    arc_radius_ = arc_size / 2;

    // Update arc
    if (arc_) {
        lv_obj_set_size(arc_, arc_size, arc_size);
        lv_obj_center(arc_);
        lv_obj_set_style_arc_width(arc_, arc_width, LV_PART_MAIN);
        lv_obj_set_style_arc_width(arc_, arc_width / 2, LV_PART_INDICATOR);
        lv_obj_set_style_pad_all(arc_, arc_width / 4, LV_PART_INDICATOR);
    }

    // Update indicator line
    if (indicator_) {
        lv_obj_set_style_line_width(indicator_, indicator_thickness, 0);
        line_points_[0].x = arc_center_x_;
        line_points_[0].y = arc_center_y_;
    }

    // Update center circles
    if (center_circle_) {
        lv_obj_set_size(center_circle_, center_circle_size, center_circle_size);
        lv_obj_center(center_circle_);
    }
    if (inner_circle_) {
        lv_obj_set_size(inner_circle_, inner_circle_size, inner_circle_size);
        lv_obj_center(inner_circle_);
    }

    // Update arc value display
    updateArc();
}

void KnobWidget::applyColors() {
    uint32_t bg = bg_color_ != 0 ? bg_color_ : BaseTheme::Color::INACTIVE;
    uint32_t track = track_color_ != 0 ? track_color_ : BaseTheme::Color::KNOB_TRACK;
    uint32_t value_col = value_color_ != 0 ? value_color_ : BaseTheme::Color::KNOB_VALUE;

    if (arc_) {
        lv_obj_set_style_arc_color(arc_, lv_color_hex(bg), LV_PART_MAIN);
        lv_obj_set_style_arc_color(arc_, lv_color_hex(track), LV_PART_INDICATOR);
    }
    if (indicator_) {
        lv_obj_set_style_line_color(indicator_, lv_color_hex(value_col), 0);
    }
    if (center_circle_) {
        lv_obj_set_style_bg_color(center_circle_, lv_color_hex(value_col), 0);
    }
}

// Fluent setters
KnobWidget& KnobWidget::centered(bool c) & {
    centered_ = c;
    if (c && origin_ == 0.0f) {
        origin_ = 0.5f;
        value_ = 0.5f;
    }
    updateArc();
    return *this;
}

KnobWidget KnobWidget::centered(bool c) && {
    return std::move(centered(c));
}

KnobWidget& KnobWidget::origin(float o) & {
    origin_ = std::clamp(o, 0.0f, 1.0f);
    updateArc();
    return *this;
}

KnobWidget KnobWidget::origin(float o) && {
    return std::move(origin(o));
}

KnobWidget& KnobWidget::bgColor(uint32_t color) & {
    bg_color_ = color;
    applyColors();
    return *this;
}

KnobWidget KnobWidget::bgColor(uint32_t color) && {
    return std::move(bgColor(color));
}

KnobWidget& KnobWidget::trackColor(uint32_t color) & {
    track_color_ = color;
    applyColors();
    return *this;
}

KnobWidget KnobWidget::trackColor(uint32_t color) && {
    return std::move(trackColor(color));
}

KnobWidget& KnobWidget::valueColor(uint32_t color) & {
    value_color_ = color;
    applyColors();
    return *this;
}

KnobWidget KnobWidget::valueColor(uint32_t color) && {
    return std::move(valueColor(color));
}

KnobWidget& KnobWidget::flashColor(uint32_t color) & {
    flash_color_ = color;
    return *this;
}

KnobWidget KnobWidget::flashColor(uint32_t color) && {
    return std::move(flashColor(color));
}

void KnobWidget::setValue(float value) {
    float clamped = std::clamp(value, 0.0f, 1.0f);
    if (std::abs(value_ - clamped) < 0.001f) return;

    value_ = clamped;
    updateArc();
    triggerFlash();
}

void KnobWidget::setVisible(bool visible) {
    if (!container_) return;
    if (visible) {
        lv_obj_clear_flag(container_, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(container_, LV_OBJ_FLAG_HIDDEN);
    }
}

void KnobWidget::updateArc() {
    if (!arc_ || !indicator_ || arc_radius_ == 0) return;

    float origin_angle = normalizedToAngle(origin_);
    float value_angle = normalizedToAngle(value_);

    if (value_ >= origin_) {
        lv_arc_set_angles(arc_, static_cast<int16_t>(origin_angle),
                         static_cast<int16_t>(value_angle));
    } else {
        lv_arc_set_angles(arc_, static_cast<int16_t>(value_angle),
                         static_cast<int16_t>(origin_angle));
    }

    float angle_rad = value_angle * static_cast<float>(M_PI) / 180.0f;
    updateIndicatorLine(angle_rad);
}

void KnobWidget::updateIndicatorLine(float angleRad) {
    float sin_val = std::sin(angleRad);
    float cos_val = std::cos(angleRad);

    line_points_[1].x = arc_center_x_ + static_cast<lv_coord_t>(arc_radius_ * cos_val);
    line_points_[1].y = arc_center_y_ + static_cast<lv_coord_t>(arc_radius_ * sin_val);

    lv_obj_refresh_self_size(indicator_);
    lv_obj_invalidate(indicator_);
}

float KnobWidget::normalizedToAngle(float normalized) const {
    return START_ANGLE + (normalized * ARC_SWEEP_DEGREES);
}

void KnobWidget::triggerFlash() {
    if (!inner_circle_) return;

    if (flash_timer_) {
        lv_timer_delete(flash_timer_);
        flash_timer_ = nullptr;
    }

    uint32_t flash = flash_color_ != 0 ? flash_color_ : BaseTheme::Color::ACTIVE;
    lv_obj_set_style_bg_color(inner_circle_, lv_color_hex(flash), 0);

    flash_timer_ = lv_timer_create(flashTimerCallback, BaseTheme::Animation::FLASH_DURATION_MS, this);
    lv_timer_set_repeat_count(flash_timer_, 1);
}

void KnobWidget::flashTimerCallback(lv_timer_t* timer) {
    auto* widget = static_cast<KnobWidget*>(lv_timer_get_user_data(timer));
    if (!widget || !widget->inner_circle_) return;

    uint32_t bg = widget->bg_color_ != 0 ? widget->bg_color_ : BaseTheme::Color::INACTIVE;
    lv_obj_set_style_bg_color(widget->inner_circle_, lv_color_hex(bg), 0);
    widget->flash_timer_ = nullptr;
}

}  // namespace oc::ui::lvgl
