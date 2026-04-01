#include <oc/ui/lvgl/widget/KnobWidget.hpp>

#include <algorithm>
#include <cmath>

#include <config/PlatformCompat.hpp>

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
      background_arc_(other.background_arc_),
      arc_(other.arc_),
      ribbon_arc_(other.ribbon_arc_),
      indicator_(other.indicator_),
      center_circle_(other.center_circle_),
      inner_circle_(other.inner_circle_),
      center_label_(other.center_label_),
      flash_timer_(other.flash_timer_),
      bg_color_(other.bg_color_),
      track_color_(other.track_color_),
      value_color_(other.value_color_),
      flash_color_(other.flash_color_),
      flash_enabled_(other.flash_enabled_),
      render_profile_(other.render_profile_),
      center_text_font_(other.center_text_font_),
      ribbon_color_(other.ribbon_color_),
      ribbon_opa_(other.ribbon_opa_),
      ribbon_thickness_ratio_(other.ribbon_thickness_ratio_),
      value_(other.value_),
      origin_(other.origin_),
      ribbon_value_(other.ribbon_value_),
      centered_(other.centered_),
      ribbon_enabled_(other.ribbon_enabled_),
      last_flash_ms_(other.last_flash_ms_),
      center_text_(std::move(other.center_text_)),
      size_policy_(other.size_policy_),
      knob_size_(other.knob_size_),
      arc_radius_(other.arc_radius_),
      indicator_thickness_(other.indicator_thickness_),
      center_x_(other.center_x_),
      center_y_(other.center_y_),
      last_layout_width_(other.last_layout_width_),
      last_layout_height_(other.last_layout_height_),
      last_arc_size_(other.last_arc_size_),
      last_arc_width_(other.last_arc_width_),
      last_ribbon_width_(other.last_ribbon_width_),
      last_indicator_size_(other.last_indicator_size_),
      last_indicator_thickness_(other.last_indicator_thickness_),
      last_center_circle_size_(other.last_center_circle_size_),
      last_inner_circle_size_(other.last_inner_circle_size_),
      last_center_label_width_(other.last_center_label_width_),
      arc_start_angle_(other.arc_start_angle_),
      arc_end_angle_(other.arc_end_angle_),
      ribbon_start_angle_(other.ribbon_start_angle_),
      ribbon_end_angle_(other.ribbon_end_angle_),
      indicator_end_x_(other.indicator_end_x_),
      indicator_end_y_(other.indicator_end_y_) {
    line_points_[0] = other.line_points_[0];
    line_points_[1] = other.line_points_[1];
    other.container_ = nullptr;
    other.background_arc_ = nullptr;
    other.arc_ = nullptr;
    other.ribbon_arc_ = nullptr;
    other.indicator_ = nullptr;
    other.center_circle_ = nullptr;
    other.inner_circle_ = nullptr;
    other.center_label_ = nullptr;
    other.flash_timer_ = nullptr;
}

KnobWidget& KnobWidget::operator=(KnobWidget&& other) noexcept {
    if (this != &other) {
        cleanup();
        container_ = other.container_;
        background_arc_ = other.background_arc_;
        arc_ = other.arc_;
        ribbon_arc_ = other.ribbon_arc_;
        indicator_ = other.indicator_;
        center_circle_ = other.center_circle_;
        inner_circle_ = other.inner_circle_;
        center_label_ = other.center_label_;
        flash_timer_ = other.flash_timer_;
        line_points_[0] = other.line_points_[0];
        line_points_[1] = other.line_points_[1];
        bg_color_ = other.bg_color_;
        track_color_ = other.track_color_;
        value_color_ = other.value_color_;
        flash_color_ = other.flash_color_;
        flash_enabled_ = other.flash_enabled_;
        render_profile_ = other.render_profile_;
        center_text_font_ = other.center_text_font_;
        ribbon_color_ = other.ribbon_color_;
        ribbon_opa_ = other.ribbon_opa_;
        ribbon_thickness_ratio_ = other.ribbon_thickness_ratio_;
        value_ = other.value_;
        origin_ = other.origin_;
        ribbon_value_ = other.ribbon_value_;
        centered_ = other.centered_;
        ribbon_enabled_ = other.ribbon_enabled_;
        last_flash_ms_ = other.last_flash_ms_;
        center_text_ = std::move(other.center_text_);
        size_policy_ = other.size_policy_;
        knob_size_ = other.knob_size_;
        arc_radius_ = other.arc_radius_;
        indicator_thickness_ = other.indicator_thickness_;
        center_x_ = other.center_x_;
        center_y_ = other.center_y_;
        last_layout_width_ = other.last_layout_width_;
        last_layout_height_ = other.last_layout_height_;
        last_arc_size_ = other.last_arc_size_;
        last_arc_width_ = other.last_arc_width_;
        last_ribbon_width_ = other.last_ribbon_width_;
        last_indicator_size_ = other.last_indicator_size_;
        last_indicator_thickness_ = other.last_indicator_thickness_;
        last_center_circle_size_ = other.last_center_circle_size_;
        last_inner_circle_size_ = other.last_inner_circle_size_;
        last_center_label_width_ = other.last_center_label_width_;
        arc_start_angle_ = other.arc_start_angle_;
        arc_end_angle_ = other.arc_end_angle_;
        ribbon_start_angle_ = other.ribbon_start_angle_;
        ribbon_end_angle_ = other.ribbon_end_angle_;
        indicator_end_x_ = other.indicator_end_x_;
        indicator_end_y_ = other.indicator_end_y_;
        other.container_ = nullptr;
        other.background_arc_ = nullptr;
        other.arc_ = nullptr;
        other.ribbon_arc_ = nullptr;
        other.indicator_ = nullptr;
        other.center_circle_ = nullptr;
        other.inner_circle_ = nullptr;
        other.center_label_ = nullptr;
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
    background_arc_ = nullptr;
    arc_ = nullptr;
    ribbon_arc_ = nullptr;
    indicator_ = nullptr;
    center_circle_ = nullptr;
    inner_circle_ = nullptr;
    center_label_ = nullptr;
}

FLASHMEM void KnobWidget::createUI() {
    lv_obj_set_style_bg_opa(container_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(container_, 0, 0);
    lv_obj_set_style_pad_all(container_, 0, 0);
    lv_obj_set_style_pad_row(container_, 0, 0);
    lv_obj_set_style_pad_column(container_, 0, 0);
    lv_obj_add_flag(container_, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    lv_obj_set_scrollbar_mode(container_, LV_SCROLLBAR_MODE_OFF);

    createBackgroundArc();
    createArc();
    createRibbon();
    createIndicator();
    createCenterCircles();
    center_label_ = lv_label_create(container_);
    lv_obj_remove_flag(center_label_, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(center_label_, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_label_set_long_mode(center_label_, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_bg_opa(center_label_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(center_label_, 0, 0);
    lv_obj_set_style_pad_all(center_label_, 0, 0);
    lv_obj_set_style_text_align(center_label_, LV_TEXT_ALIGN_CENTER, 0);
    if (center_text_font_) {
        lv_obj_set_style_text_font(center_label_, center_text_font_, 0);
    }
    lv_label_set_text(center_label_, "");
    lv_obj_add_flag(center_label_, LV_OBJ_FLAG_HIDDEN);
    applyColors();
    applyRibbonColors();
    applyRenderProfile();

    lv_obj_add_event_cb(container_, sizeChangedCallback, LV_EVENT_SIZE_CHANGED, this);

    lv_timer_t* init_timer = lv_timer_create([](lv_timer_t* t) {
        auto* widget = static_cast<KnobWidget*>(lv_timer_get_user_data(t));
        if (widget) widget->updateGeometry();
    }, 0, this);
    lv_timer_set_repeat_count(init_timer, 1);
}

FLASHMEM void KnobWidget::createBackgroundArc() {
    background_arc_ = lv_arc_create(container_);
    lv_obj_center(background_arc_);
    lv_obj_remove_flag(background_arc_, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(background_arc_, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_arc_set_bg_angles(background_arc_, START_ANGLE, END_ANGLE);
    lv_obj_remove_style(background_arc_, nullptr, LV_PART_KNOB);
    lv_obj_set_style_arc_opa(background_arc_, LV_OPA_TRANSP, LV_PART_INDICATOR);
}

FLASHMEM void KnobWidget::createArc() {
    arc_ = lv_arc_create(container_);
    lv_obj_center(arc_);
    lv_obj_remove_flag(arc_, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(arc_, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_arc_set_bg_angles(arc_, START_ANGLE, END_ANGLE);
    lv_obj_remove_style(arc_, nullptr, LV_PART_KNOB);
}

FLASHMEM void KnobWidget::createRibbon() {
    ribbon_arc_ = lv_arc_create(container_);
    lv_obj_center(ribbon_arc_);
    lv_obj_remove_flag(ribbon_arc_, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(ribbon_arc_, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_arc_set_bg_angles(ribbon_arc_, START_ANGLE, END_ANGLE);
    lv_obj_remove_style(ribbon_arc_, nullptr, LV_PART_KNOB);
    lv_obj_set_style_arc_opa(ribbon_arc_, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_add_flag(ribbon_arc_, LV_OBJ_FLAG_HIDDEN);
}

FLASHMEM void KnobWidget::createIndicator() {
    indicator_ = lv_line_create(container_);
    lv_obj_add_flag(indicator_, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_set_style_line_rounded(indicator_, true, 0);
    line_points_[0] = {0, 0};
    line_points_[1] = {0, 0};
    lv_line_set_points_mutable(indicator_, line_points_, 2);
}

FLASHMEM void KnobWidget::createCenterCircles() {
    center_circle_ = lv_obj_create(container_);
    lv_obj_center(center_circle_);
    lv_obj_set_style_radius(center_circle_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(center_circle_, 0, 0);
    lv_obj_set_style_bg_opa(center_circle_, LV_OPA_COVER, 0);
    lv_obj_set_scrollbar_mode(center_circle_, LV_SCROLLBAR_MODE_OFF);
    lv_obj_remove_flag(center_circle_, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(center_circle_, LV_OBJ_FLAG_EVENT_BUBBLE);

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

FLASHMEM void KnobWidget::applyRenderProfile() {
    const bool arc_only = render_profile_ == KnobRenderProfile::ArcOnly;

    if (background_arc_) {
        if (arc_only) lv_obj_clear_flag(background_arc_, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(background_arc_, LV_OBJ_FLAG_HIDDEN);
    }
    if (indicator_) {
        if (arc_only) lv_obj_add_flag(indicator_, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_clear_flag(indicator_, LV_OBJ_FLAG_HIDDEN);
    }
    if (center_circle_) {
        if (arc_only) lv_obj_add_flag(center_circle_, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_clear_flag(center_circle_, LV_OBJ_FLAG_HIDDEN);
    }
    if (inner_circle_) {
        if (arc_only) lv_obj_add_flag(inner_circle_, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_clear_flag(inner_circle_, LV_OBJ_FLAG_HIDDEN);
    }
    if (center_label_) {
        if (center_text_.empty()) lv_obj_add_flag(center_label_, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_clear_flag(center_label_, LV_OBJ_FLAG_HIDDEN);
    }
}

void KnobWidget::sizeChangedCallback(lv_event_t* e) {
    auto* widget = static_cast<KnobWidget*>(lv_event_get_user_data(e));
    if (widget) widget->updateGeometry();
}

FLASHMEM void KnobWidget::updateGeometry() {
    if (!container_) return;

    const auto result = size_policy_.compute(container_);
    if (!result.valid) return;

    if (result.modify_width && lv_obj_get_width(container_) != result.width) {
        lv_obj_set_width(container_, result.width);
    }
    if (result.modify_height && lv_obj_get_height(container_) != result.height) {
        lv_obj_set_height(container_, result.height);
    }

    const float min_size = static_cast<float>(std::min(result.width, result.height));
    if (min_size <= 0.0f) return;

    const float raw_size = std::max(static_cast<float>(MIN_SIZE), min_size);
    knob_size_ = static_cast<float>(static_cast<int>(raw_size) & ~1);
    center_x_ = knob_size_ / 2.0f;
    center_y_ = knob_size_ / 2.0f;

    static constexpr float ARC_RADIUS_RATIO = (1.0f - INDICATOR_RATIO) / 2.0f;
    auto make_even = [](float v) { return static_cast<lv_coord_t>(static_cast<int>(v) & ~1); };

    const float arc_width_ratio =
        render_profile_ == KnobRenderProfile::ArcOnly ? ARC_ONLY_WIDTH_RATIO : ARC_WIDTH_RATIO;
    const lv_coord_t arc_width = make_even(knob_size_ * arc_width_ratio);
    const lv_coord_t indicator_thickness = make_even(knob_size_ * INDICATOR_RATIO);
    const lv_coord_t center_circle_size = make_even(knob_size_ * CENTER_CIRCLE_RATIO);
    const lv_coord_t inner_circle_size = make_even(knob_size_ * INNER_CIRCLE_RATIO);
    arc_radius_ = knob_size_ * ARC_RADIUS_RATIO;
    indicator_thickness_ = knob_size_ * INDICATOR_RATIO;
    const lv_coord_t arc_size = make_even(arc_radius_ * 2.0f);
    const lv_coord_t ribbon_width = make_even(arc_width * ribbon_thickness_ratio_);
    const lv_coord_t indicator_size = static_cast<lv_coord_t>(knob_size_);
    const lv_coord_t center_label_width = make_even(knob_size_ * 0.62f);
    const bool layout_changed =
        last_layout_width_ != result.width ||
        last_layout_height_ != result.height;
    const bool arc_geometry_changed =
        last_arc_size_ != arc_size ||
        last_arc_width_ != arc_width;
    const bool ribbon_geometry_changed =
        last_arc_size_ != arc_size ||
        last_ribbon_width_ != ribbon_width;
    const bool indicator_geometry_changed =
        last_indicator_size_ != indicator_size ||
        last_indicator_thickness_ != indicator_thickness;
    const bool center_geometry_changed =
        last_center_circle_size_ != center_circle_size ||
        last_inner_circle_size_ != inner_circle_size ||
        last_center_label_width_ != center_label_width;

    if (!(layout_changed || arc_geometry_changed || ribbon_geometry_changed ||
          indicator_geometry_changed || center_geometry_changed)) {
        return;
    }

    if (background_arc_) {
        if (last_arc_size_ != arc_size) lv_obj_set_size(background_arc_, arc_size, arc_size);
        if (layout_changed || last_arc_size_ != arc_size) lv_obj_center(background_arc_);
        if (last_arc_width_ != arc_width) {
            lv_obj_set_style_arc_width(background_arc_, arc_width, LV_PART_MAIN);
        }
    }

    if (arc_) {
        if (last_arc_size_ != arc_size) lv_obj_set_size(arc_, arc_size, arc_size);
        if (layout_changed || last_arc_size_ != arc_size) lv_obj_center(arc_);
        if (last_arc_width_ != arc_width) {
            lv_obj_set_style_arc_width(arc_, arc_width, LV_PART_MAIN);
            if (render_profile_ == KnobRenderProfile::ArcOnly) {
                lv_obj_set_style_arc_width(arc_, arc_width, LV_PART_INDICATOR);
                lv_obj_set_style_pad_all(arc_, 0, LV_PART_INDICATOR);
            } else {
                lv_obj_set_style_arc_width(arc_, arc_width / 2, LV_PART_INDICATOR);
                lv_obj_set_style_pad_all(arc_, arc_width / 4, LV_PART_INDICATOR);
            }
        }
    }

    if (ribbon_arc_) {
        if (last_arc_size_ != arc_size) lv_obj_set_size(ribbon_arc_, arc_size, arc_size);
        if (layout_changed || last_arc_size_ != arc_size) lv_obj_center(ribbon_arc_);
        if (last_ribbon_width_ != ribbon_width) {
            lv_obj_set_style_arc_width(ribbon_arc_, ribbon_width, LV_PART_INDICATOR);
        }
    }

    if (indicator_) {
        if (last_indicator_size_ != indicator_size) {
            lv_obj_set_size(indicator_, indicator_size, indicator_size);
        }
        if (layout_changed || last_indicator_size_ != indicator_size) lv_obj_center(indicator_);
        if (last_indicator_thickness_ != indicator_thickness) {
            lv_obj_set_style_line_width(indicator_, indicator_thickness, 0);
        }
        line_points_[0].x = center_x_;
        line_points_[0].y = center_y_;
    }

    if (center_circle_) {
        if (last_center_circle_size_ != center_circle_size) {
            lv_obj_set_size(center_circle_, center_circle_size, center_circle_size);
        }
        if (layout_changed || last_center_circle_size_ != center_circle_size) lv_obj_center(center_circle_);
    }
    if (inner_circle_) {
        if (last_inner_circle_size_ != inner_circle_size) {
            lv_obj_set_size(inner_circle_, inner_circle_size, inner_circle_size);
        }
        if (layout_changed || last_inner_circle_size_ != inner_circle_size) lv_obj_center(inner_circle_);
    }
    if (center_label_) {
        if (last_center_label_width_ != center_label_width) {
            lv_obj_set_width(center_label_, center_label_width);
        }
        if (layout_changed || last_center_label_width_ != center_label_width) lv_obj_center(center_label_);
    }

    last_layout_width_ = result.width;
    last_layout_height_ = result.height;
    last_arc_size_ = arc_size;
    last_arc_width_ = arc_width;
    last_ribbon_width_ = ribbon_width;
    last_indicator_size_ = indicator_size;
    last_indicator_thickness_ = indicator_thickness;
    last_center_circle_size_ = center_circle_size;
    last_inner_circle_size_ = inner_circle_size;
    last_center_label_width_ = center_label_width;

    arc_start_angle_ = -32768;
    arc_end_angle_ = -32768;
    ribbon_start_angle_ = -32768;
    ribbon_end_angle_ = -32768;
    indicator_end_x_ = -1.0f;
    indicator_end_y_ = -1.0f;

    updateArc();
    updateRibbon();
}

FLASHMEM void KnobWidget::applyColors() {
    const uint32_t bg = bg_color_ != 0 ? bg_color_ : base_theme::color::INACTIVE;
    const uint32_t track = track_color_ != 0 ? track_color_ : base_theme::color::KNOB_TRACK;
    const uint32_t value_col = value_color_ != 0 ? value_color_ : base_theme::color::KNOB_VALUE;

    if (background_arc_) {
        lv_obj_set_style_arc_color(background_arc_, lv_color_hex(bg), LV_PART_MAIN);
        lv_obj_set_style_arc_opa(background_arc_,
                                 render_profile_ == KnobRenderProfile::ArcOnly ? LV_OPA_60 : LV_OPA_TRANSP,
                                 LV_PART_MAIN);
    }
    if (arc_) {
        lv_obj_set_style_arc_color(arc_, lv_color_hex(bg), LV_PART_MAIN);
        lv_obj_set_style_arc_opa(arc_,
                                 render_profile_ == KnobRenderProfile::ArcOnly ? LV_OPA_TRANSP : LV_OPA_COVER,
                                 LV_PART_MAIN);
        lv_obj_set_style_arc_color(arc_, lv_color_hex(track), LV_PART_INDICATOR);
    }
    if (indicator_) {
        lv_obj_set_style_line_color(indicator_, lv_color_hex(value_col), 0);
    }
    if (center_circle_) {
        lv_obj_set_style_bg_color(center_circle_, lv_color_hex(value_col), 0);
    }
    if (center_label_) {
        lv_obj_set_style_text_color(center_label_, lv_color_hex(value_col), 0);
    }
    if (inner_circle_ && !flash_enabled_) {
        lv_obj_set_style_bg_color(inner_circle_, lv_color_hex(bg), 0);
    }
}

FLASHMEM void KnobWidget::applyRibbonColors() {
    if (!ribbon_arc_) return;
    const uint32_t color = ribbon_color_ != 0 ? ribbon_color_ : base_theme::color::MACRO_6_BLUE;
    lv_obj_set_style_arc_color(ribbon_arc_, lv_color_hex(color), LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(ribbon_arc_, ribbon_opa_, LV_PART_INDICATOR);
}

KnobWidget& KnobWidget::centered(bool c) {
    if (centered_ == c) return *this;
    centered_ = c;
    if (c && origin_ == 0.0f) {
        origin_ = 0.5f;
        value_ = 0.5f;
    }
    updateArc();
    return *this;
}

KnobWidget& KnobWidget::origin(float o) {
    const float clamped = std::clamp(o, 0.0f, 1.0f);
    if (std::abs(origin_ - clamped) < 0.001f) return *this;
    origin_ = clamped;
    updateArc();
    return *this;
}

KnobWidget& KnobWidget::bgColor(uint32_t color) {
    if (bg_color_ == color) return *this;
    bg_color_ = color;
    applyColors();
    return *this;
}

KnobWidget& KnobWidget::trackColor(uint32_t color) {
    if (track_color_ == color) return *this;
    track_color_ = color;
    applyColors();
    return *this;
}

KnobWidget& KnobWidget::valueColor(uint32_t color) {
    if (value_color_ == color) return *this;
    value_color_ = color;
    applyColors();
    return *this;
}

KnobWidget& KnobWidget::flashColor(uint32_t color) {
    flash_color_ = color;
    return *this;
}

KnobWidget& KnobWidget::flashEnabled(bool enabled) {
    if (flash_enabled_ == enabled) return *this;
    flash_enabled_ = enabled;
    if (!flash_enabled_) {
        if (flash_timer_) {
            lv_timer_delete(flash_timer_);
            flash_timer_ = nullptr;
        }
        if (inner_circle_) {
            const uint32_t bg = bg_color_ != 0 ? bg_color_ : base_theme::color::INACTIVE;
            lv_obj_set_style_bg_color(inner_circle_, lv_color_hex(bg), 0);
        }
    }
    return *this;
}

KnobWidget& KnobWidget::renderProfile(KnobRenderProfile profile) {
    if (render_profile_ == profile) return *this;
    render_profile_ = profile;
    applyRenderProfile();
    applyColors();
    updateGeometry();
    return *this;
}

KnobWidget& KnobWidget::centerTextFont(const lv_font_t* font) {
    if (center_text_font_ == font) return *this;
    center_text_font_ = font;
    if (center_label_ && center_text_font_) {
        lv_obj_set_style_text_font(center_label_, center_text_font_, 0);
    }
    return *this;
}

KnobWidget& KnobWidget::ribbonColor(uint32_t color) {
    if (ribbon_color_ == color) return *this;
    ribbon_color_ = color;
    applyRibbonColors();
    return *this;
}

KnobWidget& KnobWidget::ribbonOpacity(lv_opa_t opa) {
    if (ribbon_opa_ == opa) return *this;
    ribbon_opa_ = opa;
    applyRibbonColors();
    return *this;
}

KnobWidget& KnobWidget::ribbonThickness(float ratio) {
    const float clamped = std::clamp(ratio, 0.1f, 1.0f);
    if (std::abs(ribbon_thickness_ratio_ - clamped) < 0.001f) return *this;
    ribbon_thickness_ratio_ = clamped;
    updateGeometry();
    return *this;
}

KnobWidget& KnobWidget::sizeMode(SizeMode mode) {
    if (size_policy_.mode == mode) return *this;
    size_policy_.mode = mode;
    updateGeometry();
    return *this;
}

void KnobWidget::setValue(float value) {
    const float clamped = std::clamp(value, 0.0f, 1.0f);
    if (std::abs(value_ - clamped) < 0.001f) return;

    value_ = clamped;
    updateArc();
    if (flash_enabled_) {
        triggerFlash();
    }
}

void KnobWidget::setVisible(bool visible) {
    if (!container_) return;
    if (visible) lv_obj_clear_flag(container_, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(container_, LV_OBJ_FLAG_HIDDEN);
}

void KnobWidget::setCenterText(const char* text) {
    const char* safe = text ? text : "";
    if (center_text_ == safe) return;

    center_text_ = safe;
    if (!center_label_) return;

    lv_label_set_text(center_label_, center_text_.c_str());
    if (center_text_.empty()) lv_obj_add_flag(center_label_, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_clear_flag(center_label_, LV_OBJ_FLAG_HIDDEN);
}

void KnobWidget::setRibbonValue(float value) {
    const float clamped = std::clamp(value, 0.0f, 1.0f);
    if (std::abs(ribbon_value_ - clamped) < 0.001f && ribbon_enabled_) return;

    ribbon_value_ = clamped;
    if (!ribbon_enabled_) {
        ribbon_enabled_ = true;
        lv_obj_clear_flag(ribbon_arc_, LV_OBJ_FLAG_HIDDEN);
    }
    updateRibbon();
}

void KnobWidget::setRibbonEnabled(bool enabled) {
    if (ribbon_enabled_ == enabled) return;

    ribbon_enabled_ = enabled;
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

    const int16_t value_angle = static_cast<int16_t>(std::round(normalizedToAngle(value_)));
    const int16_t ribbon_angle = static_cast<int16_t>(std::round(normalizedToAngle(ribbon_value_)));
    const int16_t start = std::min(value_angle, ribbon_angle);
    const int16_t end = std::max(value_angle, ribbon_angle);

    if (ribbon_start_angle_ == start && ribbon_end_angle_ == end) return;

    lv_arc_set_angles(ribbon_arc_, start, end);
    ribbon_start_angle_ = start;
    ribbon_end_angle_ = end;
}

bool KnobWidget::updateArc() {
    if (!arc_ || arc_radius_ <= 0.0f) return false;

    const int16_t origin_angle = static_cast<int16_t>(std::round(normalizedToAngle(origin_)));
    const int16_t value_angle = static_cast<int16_t>(std::round(normalizedToAngle(value_)));
    const int16_t start = std::min(origin_angle, value_angle);
    const int16_t end = std::max(origin_angle, value_angle);

    bool updated = false;
    if (arc_start_angle_ != start || arc_end_angle_ != end) {
        lv_arc_set_angles(arc_, start, end);
        arc_start_angle_ = start;
        arc_end_angle_ = end;
        updated = true;
    }

    if (render_profile_ != KnobRenderProfile::ArcOnly) {
        const float angle_rad = static_cast<float>(value_angle) * static_cast<float>(M_PI) / 180.0f;
        updated = updateIndicatorLine(angle_rad) || updated;
    }
    return updated;
}

bool KnobWidget::updateIndicatorLine(float angleRad) {
    const float end_x = center_x_ + arc_radius_ * std::cos(angleRad);
    const float end_y = center_y_ + arc_radius_ * std::sin(angleRad);

    if (std::abs(indicator_end_x_ - end_x) < 0.25f &&
        std::abs(indicator_end_y_ - end_y) < 0.25f) {
        return false;
    }

    line_points_[1].x = end_x;
    line_points_[1].y = end_y;
    indicator_end_x_ = end_x;
    indicator_end_y_ = end_y;
    lv_obj_invalidate(indicator_);
    return true;
}

float KnobWidget::normalizedToAngle(float normalized) const {
    return START_ANGLE + (normalized * ARC_SWEEP_DEGREES);
}

FLASHMEM void KnobWidget::triggerFlash() {
    if (!flash_enabled_ || !inner_circle_ || render_profile_ == KnobRenderProfile::ArcOnly) return;

    const uint32_t now = lv_tick_get();
    if (now - last_flash_ms_ < FLASH_RATE_LIMIT_MS) return;
    last_flash_ms_ = now;

    if (flash_timer_) {
        lv_timer_delete(flash_timer_);
        flash_timer_ = nullptr;
    }

    const uint32_t flash = flash_color_ != 0 ? flash_color_ : base_theme::color::ACTIVE;
    lv_obj_set_style_bg_color(inner_circle_, lv_color_hex(flash), 0);

    flash_timer_ = lv_timer_create(flashTimerCallback, base_theme::animation::FLASH_DURATION_MS, this);
    lv_timer_set_repeat_count(flash_timer_, 1);
}

FLASHMEM void KnobWidget::flashTimerCallback(lv_timer_t* timer) {
    auto* widget = static_cast<KnobWidget*>(lv_timer_get_user_data(timer));
    if (!widget || !widget->inner_circle_) return;

    const uint32_t bg = widget->bg_color_ != 0 ? widget->bg_color_ : base_theme::color::INACTIVE;
    lv_obj_set_style_bg_color(widget->inner_circle_, lv_color_hex(bg), 0);
    widget->flash_timer_ = nullptr;
}

}  // namespace oc::ui::lvgl
