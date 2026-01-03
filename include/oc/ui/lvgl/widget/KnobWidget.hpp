#pragma once

#include <cstdint>

#include <lvgl.h>

#include <oc/ui/lvgl/IWidget.hpp>
#include <oc/ui/lvgl/SquareSizePolicy.hpp>

#include "../theme/BaseTheme.hpp"

namespace oc::ui::lvgl {

/**
 * @brief Rotary knob widget with arc visualization
 *
 * Displays a parameter value as a circular arc with indicator line.
 * Supports normal and centered (bipolar) modes.
 * Pure visual widget - input handling is external.
 * Flashes inner circle on value change.
 *
 * The widget adapts to its parent size:
 * - Takes 100% of parent width/height
 * - Uses min(width, height) for knob size (always square)
 * - Minimum size: 30px
 * - Centers the knob within the container
 *
 * Usage:
 * @code
 * KnobWidget knob(parent);
 * knob.trackColor(0xFCEB23)
 *     .valueColor(0x909090)
 *     .flashColor(0xECA747);
 * knob.setValue(0.5f);
 * @endcode
 */
class KnobWidget : public IWidget {
public:
    explicit KnobWidget(lv_obj_t* parent);
    ~KnobWidget();

    // Move only
    KnobWidget(KnobWidget&& other) noexcept;
    KnobWidget& operator=(KnobWidget&& other) noexcept;
    KnobWidget(const KnobWidget&) = delete;
    KnobWidget& operator=(const KnobWidget&) = delete;

    // LVGL Access
    lv_obj_t* getElement() const override { return container_; }
    operator lv_obj_t*() const { return container_; }

    // Fluent Configuration
    KnobWidget& centered(bool c);
    KnobWidget& origin(float o);
    KnobWidget& bgColor(uint32_t color);     ///< Arc background (unfilled)
    KnobWidget& trackColor(uint32_t color);  ///< Arc filled portion
    KnobWidget& valueColor(uint32_t color);  ///< Indicator line + center
    KnobWidget& flashColor(uint32_t color);  ///< Flash on value change

    // Ribbon Configuration (modulated value arc)
    KnobWidget& ribbonColor(uint32_t color);         ///< Ribbon arc color
    KnobWidget& ribbonOpacity(lv_opa_t opa);         ///< Ribbon opacity (default: LV_OPA_COVER)
    KnobWidget& ribbonThickness(float ratio);        ///< Thickness relative to main arc (0.0-1.0, default: 0.8)

    // Size Policy
    KnobWidget& sizeMode(SizeMode mode);             ///< Set sizing mode (default: Auto)

    // Data
    void setValue(float value);
    float getValue() const { return value_; }
    void setRibbonValue(float value);                ///< Set ribbon position (auto-enables ribbon)
    void setRibbonEnabled(bool enabled);             ///< Show/hide ribbon arc
    void setVisible(bool visible);

private:
    // Fixed proportions (relative to knob size)
    static constexpr uint16_t MIN_SIZE = 30;
    static constexpr float ARC_WIDTH_RATIO = 0.13f;        // Arc width as ratio of size
    static constexpr float INDICATOR_RATIO = 0.13f;        // Indicator thickness ratio
    static constexpr float CENTER_CIRCLE_RATIO = 0.22f;    // Center circle size ratio
    static constexpr float INNER_CIRCLE_RATIO = 0.10f;     // Inner circle size ratio
    static constexpr int16_t START_ANGLE = 135;
    static constexpr int16_t END_ANGLE = 45;
    static constexpr float ARC_SWEEP_DEGREES = 270.0f;

    void createUI();
    void createArc();
    void createRibbon();
    void createIndicator();
    void createCenterCircles();
    void applyColors();
    void applyRibbonColors();
    void updateGeometry();
    void updateArc();
    void updateRibbon();
    void updateIndicatorLine(float angleRad);
    void triggerFlash();
    static void flashTimerCallback(lv_timer_t* timer);
    static void sizeChangedCallback(lv_event_t* e);
    float normalizedToAngle(float normalized) const;
    void cleanup();

    // LVGL objects
    lv_obj_t* container_ = nullptr;
    lv_obj_t* arc_ = nullptr;
    lv_obj_t* ribbon_arc_ = nullptr;
    lv_obj_t* indicator_ = nullptr;
    lv_obj_t* center_circle_ = nullptr;
    lv_obj_t* inner_circle_ = nullptr;
    lv_timer_t* flash_timer_ = nullptr;

    // Indicator line points
    lv_point_precise_t line_points_[2];

    // Configuration
    uint32_t bg_color_ = 0;
    uint32_t track_color_ = 0;
    uint32_t value_color_ = 0;
    uint32_t flash_color_ = 0;

    // Ribbon configuration
    uint32_t ribbon_color_ = 0;
    lv_opa_t ribbon_opa_ = LV_OPA_COVER;
    float ribbon_thickness_ratio_ = 0.8f;

    // State
    float value_ = 0.0f;
    float origin_ = 0.0f;
    float ribbon_value_ = 0.0f;
    bool centered_ = false;
    bool ribbon_enabled_ = false;

    // Size policy
    SquareSizePolicy size_policy_;

    // Cached geometry (computed from actual size) - all float for precision
    float knob_size_ = 0.0f;
    float arc_radius_ = 0.0f;
    float indicator_thickness_ = 0.0f;
    float center_x_ = 0.0f;  // container width / 2
    float center_y_ = 0.0f;  // container height / 2
};

}  // namespace oc::ui::lvgl
