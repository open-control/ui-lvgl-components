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
      ribbon_arc_(other.ribbon_arc_),
      indicator_(other.indicator_),
      center_circle_(other.center_circle_),
      inner_circle_(other.inner_circle_),
      flash_timer_(other.flash_timer_),
      bg_color_(other.bg_color_),
      track_color_(other.track_color_),
      value_color_(other.value_color_),
      flash_color_(other.flash_color_),
      ribbon_color_(other.ribbon_color_),
      ribbon_opa_(other.ribbon_opa_),
      ribbon_thickness_ratio_(other.ribbon_thickness_ratio_),
      value_(other.value_),
      origin_(other.origin_),
      ribbon_value_(other.ribbon_value_),
      centered_(other.centered_),
      ribbon_enabled_(other.ribbon_enabled_),
      size_policy_(other.size_policy_),
      knob_size_(other.knob_size_),
      arc_radius_(other.arc_radius_),
      indicator_thickness_(other.indicator_thickness_),
      center_x_(other.center_x_),
      center_y_(other.center_y_) {
    line_points_[0] = other.line_points_[0];
    line_points_[1] = other.line_points_[1];
    other.container_ = nullptr;
    other.arc_ = nullptr;
    other.ribbon_arc_ = nullptr;
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
        ribbon_arc_ = other.ribbon_arc_;
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
        ribbon_color_ = other.ribbon_color_;
        ribbon_opa_ = other.ribbon_opa_;
        ribbon_thickness_ratio_ = other.ribbon_thickness_ratio_;
        value_ = other.value_;
        origin_ = other.origin_;
        ribbon_value_ = other.ribbon_value_;
        centered_ = other.centered_;
        ribbon_enabled_ = other.ribbon_enabled_;
        size_policy_ = other.size_policy_;
        knob_size_ = other.knob_size_;
        arc_radius_ = other.arc_radius_;
        indicator_thickness_ = other.indicator_thickness_;
        center_x_ = other.center_x_;
        center_y_ = other.center_y_;
        other.container_ = nullptr;
        other.arc_ = nullptr;
        other.ribbon_arc_ = nullptr;
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
    ribbon_arc_ = nullptr;
    indicator_ = nullptr;
    center_circle_ = nullptr;
    inner_circle_ = nullptr;
}

void KnobWidget::createUI() {
    // Size will be controlled by parent (flex/grid)
    lv_obj_set_style_bg_opa(container_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(container_, 0, 0);
    lv_obj_set_style_pad_all(container_, 0, 0);
    lv_obj_set_style_pad_row(container_, 0, 0);
    lv_obj_set_style_pad_column(container_, 0, 0);
    lv_obj_add_flag(container_, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    lv_obj_set_scrollbar_mode(container_, LV_SCROLLBAR_MODE_OFF);

    createArc();
    // ribbon_arc created lazily in setRibbonValue()/setRibbonEnabled()
    createIndicator();
    createCenterCircles();
    applyColors();

    // Listen for own size changes to recalculate geometry
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

void KnobWidget::createRibbon() {
    ribbon_arc_ = lv_arc_create(container_);
    lv_obj_center(ribbon_arc_);
    lv_obj_remove_flag(ribbon_arc_, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(ribbon_arc_, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_arc_set_bg_angles(ribbon_arc_, START_ANGLE, END_ANGLE);
    lv_obj_remove_style(ribbon_arc_, nullptr, LV_PART_KNOB);
    // Hide background arc (only show indicator part)
    lv_obj_set_style_arc_opa(ribbon_arc_, LV_OPA_TRANSP, LV_PART_MAIN);
    // Hidden by default
    lv_obj_add_flag(ribbon_arc_, LV_OBJ_FLAG_HIDDEN);
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
    lv_obj_set_scrollbar_mode(center_circle_, LV_SCROLLBAR_MODE_OFF);
    lv_obj_remove_flag(center_circle_, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(center_circle_, LV_OBJ_FLAG_EVENT_BUBBLE);

    // Inner circle (flashes on value change)
    inner_circle_ = lv_obj_create(container_);
    lv_obj_center(inner_circle_);
    lv_obj_set_style_radius(inner_circle_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(inner_circle_, 0, 0);
    lv_obj_set_style_bg_color(inner_circle_, lv_color_hex(base_theme::color::INACTIVE), 0);
    lv_obj_set_style_bg_opa(inner_circle_, LV_OPA_COVER, 0);
    lv_obj_set_scrollbar_mode(inner_circle_, LV_SCROLLBAR_MODE_OFF);
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

    // Compute size using policy
    auto result = size_policy_.compute(container_);
    if (!result.valid) return;

    // Apply container modifications if needed
    if (result.modify_width) {
        lv_obj_set_width(container_, result.width);
    }
    if (result.modify_height) {
        lv_obj_set_height(container_, result.height);
    }

    // Calculate square size from result
    float min_size = static_cast<float>(std::min(result.width, result.height));
    if (min_size <= 0.0f) return;

    // Square knob, enforce minimum
    // Round down to even number for perfect centering (int division by 2)
    float raw_size = std::max(static_cast<float>(MIN_SIZE), min_size);
    knob_size_ = static_cast<float>(static_cast<int>(raw_size) & ~1);  // Clear LSB = even

    center_x_ = knob_size_ / 2.0f;  // Knob center for indicator pivot
    center_y_ = knob_size_ / 2.0f;

    // All sizes proportional to knob_size_
    // Force even sizes for perfect centering with lv_obj_center()
    static constexpr float ARC_RADIUS_RATIO = (1.0f - INDICATOR_RATIO) / 2.0f;
    auto make_even = [](float v) { return static_cast<lv_coord_t>(static_cast<int>(v) & ~1); };

    lv_coord_t arc_width = make_even(knob_size_ * ARC_WIDTH_RATIO);
    lv_coord_t indicator_thickness = make_even(knob_size_ * INDICATOR_RATIO);
    lv_coord_t center_circle_size = make_even(knob_size_ * CENTER_CIRCLE_RATIO);
    lv_coord_t inner_circle_size = make_even(knob_size_ * INNER_CIRCLE_RATIO);
    arc_radius_ = knob_size_ * ARC_RADIUS_RATIO;
    indicator_thickness_ = knob_size_ * INDICATOR_RATIO;
    lv_coord_t arc_size = make_even(arc_radius_ * 2.0f);

    // Update arc
    if (arc_) {
        lv_obj_set_size(arc_, arc_size, arc_size);
        lv_obj_center(arc_);
        lv_obj_set_style_arc_width(arc_, arc_width, LV_PART_MAIN);
        lv_obj_set_style_arc_width(arc_, arc_width / 2, LV_PART_INDICATOR);
        lv_obj_set_style_pad_all(arc_, arc_width / 4, LV_PART_INDICATOR);
    }

    // Update ribbon arc (same size as main arc, different thickness)
    if (ribbon_arc_) {
        lv_obj_set_size(ribbon_arc_, arc_size, arc_size);
        lv_obj_center(ribbon_arc_);
        lv_coord_t ribbon_width = make_even(arc_width * ribbon_thickness_ratio_);
        lv_obj_set_style_arc_width(ribbon_arc_, ribbon_width, LV_PART_INDICATOR);
    }

    // Update indicator line
    if (indicator_) {
        lv_obj_set_style_line_width(indicator_, indicator_thickness, 0);
        line_points_[0].x = center_x_;
        line_points_[0].y = center_y_;
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
    updateRibbon();
}

void KnobWidget::applyColors() {
    uint32_t bg = bg_color_ != 0 ? bg_color_ : base_theme::color::INACTIVE;
    uint32_t track = track_color_ != 0 ? track_color_ : base_theme::color::KNOB_TRACK;
    uint32_t value_col = value_color_ != 0 ? value_color_ : base_theme::color::KNOB_VALUE;

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

void KnobWidget::applyRibbonColors() {
    if (!ribbon_arc_) return;
    uint32_t color = ribbon_color_ != 0 ? ribbon_color_ : base_theme::color::MACRO_6_BLUE;
    lv_obj_set_style_arc_color(ribbon_arc_, lv_color_hex(color), LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(ribbon_arc_, ribbon_opa_, LV_PART_INDICATOR);
}

// Fluent setters
KnobWidget& KnobWidget::centered(bool c) {
    centered_ = c;
    if (c && origin_ == 0.0f) {
        origin_ = 0.5f;
        value_ = 0.5f;
    }
    updateArc();
    return *this;
}

KnobWidget& KnobWidget::origin(float o) {
    origin_ = std::clamp(o, 0.0f, 1.0f);
    updateArc();
    return *this;
}

KnobWidget& KnobWidget::bgColor(uint32_t color) {
    bg_color_ = color;
    applyColors();
    return *this;
}

KnobWidget& KnobWidget::trackColor(uint32_t color) {
    track_color_ = color;
    applyColors();
    return *this;
}

KnobWidget& KnobWidget::valueColor(uint32_t color) {
    value_color_ = color;
    applyColors();
    return *this;
}

KnobWidget& KnobWidget::flashColor(uint32_t color) {
    flash_color_ = color;
    return *this;
}

KnobWidget& KnobWidget::ribbonColor(uint32_t color) {
    ribbon_color_ = color;
    applyRibbonColors();
    return *this;
}

KnobWidget& KnobWidget::ribbonOpacity(lv_opa_t opa) {
    ribbon_opa_ = opa;
    applyRibbonColors();
    return *this;
}

KnobWidget& KnobWidget::ribbonThickness(float ratio) {
    ribbon_thickness_ratio_ = std::clamp(ratio, 0.1f, 1.0f);
    updateGeometry();
    return *this;
}

KnobWidget& KnobWidget::sizeMode(SizeMode mode) {
    size_policy_.mode = mode;
    updateGeometry();
    return *this;
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

void KnobWidget::setRibbonValue(float value) {
    float clamped = std::clamp(value, 0.0f, 1.0f);
    ribbon_value_ = clamped;
    // Lazy-create ribbon arc on first use
    if (!ribbon_arc_) {
        createRibbon();
        applyRibbonColors();
        updateGeometry();  // Apply sizing to newly created arc
    }
    // Auto-enable ribbon when value is set
    if (!ribbon_enabled_) {
        ribbon_enabled_ = true;
        lv_obj_clear_flag(ribbon_arc_, LV_OBJ_FLAG_HIDDEN);
    }
    updateRibbon();
}

void KnobWidget::setRibbonEnabled(bool enabled) {
    ribbon_enabled_ = enabled;
    if (enabled && !ribbon_arc_) {
        // Lazy-create ribbon arc
        createRibbon();
        applyRibbonColors();
        updateGeometry();
    }
    if (!ribbon_arc_) return;
    if (enabled) {
        lv_obj_clear_flag(ribbon_arc_, LV_OBJ_FLAG_HIDDEN);
        updateRibbon();
    } else {
        lv_obj_add_flag(ribbon_arc_, LV_OBJ_FLAG_HIDDEN);
    }
}

void KnobWidget::updateRibbon() {
    if (!ribbon_arc_ || !ribbon_enabled_ || arc_radius_ <= 0.0f) return;

    float value_angle = normalizedToAngle(value_);
    float ribbon_angle = normalizedToAngle(ribbon_value_);

    // Ribbon shows between value and ribbon_value
    if (ribbon_value_ >= value_) {
        lv_arc_set_angles(ribbon_arc_, static_cast<int16_t>(value_angle),
                         static_cast<int16_t>(ribbon_angle));
    } else {
        lv_arc_set_angles(ribbon_arc_, static_cast<int16_t>(ribbon_angle),
                         static_cast<int16_t>(value_angle));
    }
}

void KnobWidget::updateArc() {
    if (!arc_ || !indicator_ || arc_radius_ <= 0.0f) return;

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
    // All calculations in float for precision
    float end_x = center_x_ + arc_radius_ * std::cos(angleRad);
    float end_y = center_y_ + arc_radius_ * std::sin(angleRad);

    // Update line endpoint (lv_point_precise_t uses float)
    line_points_[1].x = end_x;
    line_points_[1].y = end_y;
    lv_obj_refresh_self_size(indicator_);
}

float KnobWidget::normalizedToAngle(float normalized) const {
    return START_ANGLE + (normalized * ARC_SWEEP_DEGREES);
}

void KnobWidget::triggerFlash() {
    if (!inner_circle_) return;

    // Rate-limit flash to avoid excessive timer creation during rapid encoder movement
    uint32_t now = lv_tick_get();
    if (now - last_flash_ms_ < FLASH_RATE_LIMIT_MS) return;
    last_flash_ms_ = now;

    if (flash_timer_) {
        lv_timer_delete(flash_timer_);
        flash_timer_ = nullptr;
    }

    uint32_t flash = flash_color_ != 0 ? flash_color_ : base_theme::color::ACTIVE;
    lv_obj_set_style_bg_color(inner_circle_, lv_color_hex(flash), 0);

    flash_timer_ = lv_timer_create(flashTimerCallback, base_theme::animation::FLASH_DURATION_MS, this);
    lv_timer_set_repeat_count(flash_timer_, 1);
}

void KnobWidget::flashTimerCallback(lv_timer_t* timer) {
    auto* widget = static_cast<KnobWidget*>(lv_timer_get_user_data(timer));
    if (!widget || !widget->inner_circle_) return;

    uint32_t bg = widget->bg_color_ != 0 ? widget->bg_color_ : base_theme::color::INACTIVE;
    lv_obj_set_style_bg_color(widget->inner_circle_, lv_color_hex(bg), 0);
    widget->flash_timer_ = nullptr;
}

}  // namespace oc::ui::lvgl
