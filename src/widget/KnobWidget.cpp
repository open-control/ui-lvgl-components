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
      width_(other.width_),
      height_(other.height_),
      color_index_(other.color_index_),
      track_color_(other.track_color_),
      value_color_(other.value_color_),
      value_(other.value_),
      origin_(other.origin_),
      centered_(other.centered_),
      drag_start_y_(other.drag_start_y_),
      arc_center_x_(other.arc_center_x_),
      arc_center_y_(other.arc_center_y_) {
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
        width_ = other.width_;
        height_ = other.height_;
        color_index_ = other.color_index_;
        track_color_ = other.track_color_;
        value_color_ = other.value_color_;
        value_ = other.value_;
        origin_ = other.origin_;
        centered_ = other.centered_;
        drag_start_y_ = other.drag_start_y_;
        arc_center_x_ = other.arc_center_x_;
        arc_center_y_ = other.arc_center_y_;

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
    // Add padding for indicator thickness to prevent clipping
    uint16_t padded_w = width_ + INDICATOR_THICKNESS;
    uint16_t padded_h = height_ + INDICATOR_THICKNESS;
    lv_obj_set_size(container_, padded_w, padded_h);
    lv_obj_set_style_bg_opa(container_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(container_, 0, 0);
    lv_obj_set_style_pad_all(container_, 0, 0);
    lv_obj_add_flag(container_, LV_OBJ_FLAG_OVERFLOW_VISIBLE);

    arc_center_x_ = padded_w / 2;
    arc_center_y_ = padded_h / 2;

    createArc();
    createIndicator();
    createCenterCircles();
    setupDragInteraction();

    updateArc();
}

void KnobWidget::createArc() {
    arc_ = lv_arc_create(container_);
    lv_obj_set_size(arc_, ARC_SIZE, ARC_SIZE);
    lv_obj_center(arc_);
    lv_obj_remove_flag(arc_, LV_OBJ_FLAG_CLICKABLE);  // Let events pass to container
    lv_obj_add_flag(arc_, LV_OBJ_FLAG_EVENT_BUBBLE);

    lv_arc_set_bg_angles(arc_, START_ANGLE, END_ANGLE);

    // Main arc style (background)
    lv_obj_set_style_arc_width(arc_, ARC_WIDTH, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc_, lv_color_hex(BaseTheme::Color::INACTIVE), LV_PART_MAIN);

    // Indicator arc style (value track)
    lv_obj_set_style_arc_width(arc_, ARC_WIDTH / 2, LV_PART_INDICATOR);
    lv_obj_set_style_pad_all(arc_, ARC_WIDTH / 4, LV_PART_INDICATOR);

    // Remove knob (we use custom indicator line)
    lv_obj_remove_style(arc_, nullptr, LV_PART_KNOB);

    applyColors();
}

void KnobWidget::createIndicator() {
    indicator_ = lv_line_create(container_);
    lv_obj_add_flag(indicator_, LV_OBJ_FLAG_EVENT_BUBBLE);

    lv_obj_set_style_line_width(indicator_, INDICATOR_THICKNESS, 0);
    lv_obj_set_style_line_rounded(indicator_, true, 0);

    // Initialize line points at center
    line_points_[0].x = arc_center_x_;
    line_points_[0].y = arc_center_y_;
    line_points_[1].x = arc_center_x_;
    line_points_[1].y = arc_center_y_;

    lv_line_set_points_mutable(indicator_, line_points_, 2);
    applyColors();
}

void KnobWidget::createCenterCircles() {
    lv_coord_t center_y = arc_center_y_ - CENTER_CIRCLE_SIZE / 2;
    lv_coord_t inner_y = arc_center_y_ - INNER_CIRCLE_SIZE / 2;

    // Outer circle
    center_circle_ = lv_obj_create(container_);
    lv_obj_set_size(center_circle_, CENTER_CIRCLE_SIZE, CENTER_CIRCLE_SIZE);
    lv_obj_align(center_circle_, LV_ALIGN_TOP_MID, 0, center_y);
    lv_obj_set_style_radius(center_circle_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(center_circle_, 0, 0);
    lv_obj_set_style_bg_opa(center_circle_, LV_OPA_COVER, 0);
    lv_obj_remove_flag(center_circle_, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(center_circle_, LV_OBJ_FLAG_EVENT_BUBBLE);

    // Inner circle (flashes on value change)
    inner_circle_ = lv_obj_create(container_);
    lv_obj_set_size(inner_circle_, INNER_CIRCLE_SIZE, INNER_CIRCLE_SIZE);
    lv_obj_align(inner_circle_, LV_ALIGN_TOP_MID, 0, inner_y);
    lv_obj_set_style_radius(inner_circle_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(inner_circle_, 0, 0);
    lv_obj_set_style_bg_color(inner_circle_, lv_color_hex(BaseTheme::Color::INACTIVE), 0);
    lv_obj_set_style_bg_opa(inner_circle_, LV_OPA_COVER, 0);
    lv_obj_remove_flag(inner_circle_, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(inner_circle_, LV_OBJ_FLAG_EVENT_BUBBLE);

    applyColors();
}

void KnobWidget::setupDragInteraction() {
    lv_obj_add_flag(container_, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(container_, dragEventCallback, LV_EVENT_PRESSED, this);
    lv_obj_add_event_cb(container_, dragEventCallback, LV_EVENT_PRESSING, this);
}

void KnobWidget::dragEventCallback(lv_event_t* e) {
    auto* widget = static_cast<KnobWidget*>(lv_event_get_user_data(e));
    if (!widget) return;

    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_PRESSED) {
        lv_point_t p;
        lv_indev_get_point(lv_indev_active(), &p);
        widget->drag_start_y_ = p.y;
    }
    else if (code == LV_EVENT_PRESSING) {
        lv_point_t p;
        lv_indev_get_point(lv_indev_active(), &p);
        float delta = (widget->drag_start_y_ - p.y) / 100.0f;
        widget->drag_start_y_ = p.y;

        float new_value = widget->value_ + delta;
        new_value = std::clamp(new_value, 0.0f, 1.0f);
        widget->setValue(new_value);
    }
}

void KnobWidget::applyColors() {
    // Determine track color: colorIndex takes priority, then track_color_, then default
    uint32_t track;
    if (color_index_ < 8) {
        track = BaseTheme::Color::getMacroColor(color_index_);
    } else if (track_color_ != 0) {
        track = track_color_;
    } else {
        track = BaseTheme::Color::KNOB_TRACK;
    }

    // Determine value/indicator color
    uint32_t value_col = value_color_ != 0 ? value_color_ : BaseTheme::Color::KNOB_VALUE;

    if (arc_) {
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
KnobWidget& KnobWidget::size(uint16_t w, uint16_t h) & {
    width_ = w;
    height_ = h;
    if (container_) {
        lv_obj_set_size(container_, w, h);
        arc_center_x_ = w / 2;
        arc_center_y_ = h / 2;
    }
    return *this;
}

KnobWidget KnobWidget::size(uint16_t w, uint16_t h) && {
    return std::move(size(w, h));
}

KnobWidget& KnobWidget::colorIndex(uint8_t index) & {
    color_index_ = index;
    applyColors();
    return *this;
}

KnobWidget KnobWidget::colorIndex(uint8_t index) && {
    return std::move(colorIndex(index));
}

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
    if (!arc_ || !indicator_) return;

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

    line_points_[1].x = arc_center_x_ + static_cast<lv_coord_t>(ARC_RADIUS * cos_val);
    line_points_[1].y = arc_center_y_ + static_cast<lv_coord_t>(ARC_RADIUS * sin_val);

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

    lv_obj_set_style_bg_color(inner_circle_, lv_color_hex(BaseTheme::Color::ACTIVE), 0);

    flash_timer_ = lv_timer_create(flashTimerCallback, BaseTheme::Animation::FLASH_DURATION_MS, this);
    lv_timer_set_repeat_count(flash_timer_, 1);
}

void KnobWidget::flashTimerCallback(lv_timer_t* timer) {
    auto* widget = static_cast<KnobWidget*>(lv_timer_get_user_data(timer));
    if (!widget || !widget->inner_circle_) return;

    lv_obj_set_style_bg_color(widget->inner_circle_, lv_color_hex(BaseTheme::Color::INACTIVE), 0);
    widget->flash_timer_ = nullptr;
}

}  // namespace oc::ui::lvgl
