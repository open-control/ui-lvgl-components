#pragma once

#include <cstdint>

#include <lvgl.h>

#include <oc/ui/lvgl/IWidget.hpp>

#include "../theme/BaseTheme.hpp"

namespace oc::ui::lvgl {

/**
 * @brief Rotary knob widget with arc visualization
 *
 * Displays a parameter value as a circular arc with indicator line.
 * Supports normal and centered (bipolar) modes.
 * Built-in drag interaction for value control.
 *
 * Usage:
 * @code
 * KnobWidget knob(parent);
 * knob.colorIndex(2);  // Yellow macro color
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
    lv_obj_t* getArc() const { return arc_; }
    lv_obj_t* getIndicator() const { return indicator_; }
    lv_obj_t* getCenterCircle() const { return center_circle_; }

    // Fluent Configuration
    KnobWidget& size(uint16_t w, uint16_t h) &;
    KnobWidget size(uint16_t w, uint16_t h) &&;

    /** @brief Set color index (0-7 for macro colors), applies to track */
    KnobWidget& colorIndex(uint8_t index) &;
    KnobWidget colorIndex(uint8_t index) &&;

    KnobWidget& centered(bool c) &;
    KnobWidget centered(bool c) &&;

    KnobWidget& origin(float o) &;
    KnobWidget origin(float o) &&;

    KnobWidget& trackColor(uint32_t color) &;
    KnobWidget trackColor(uint32_t color) &&;

    KnobWidget& valueColor(uint32_t color) &;
    KnobWidget valueColor(uint32_t color) &&;

    // Data
    void setValue(float value);
    float getValue() const { return value_; }
    void setVisible(bool visible);

private:
    static constexpr uint16_t DEFAULT_SIZE = 62;
    static constexpr uint16_t ARC_SIZE = 62;
    static constexpr uint16_t ARC_RADIUS = ARC_SIZE / 2;
    static constexpr uint8_t ARC_WIDTH = 8;
    static constexpr uint8_t INDICATOR_THICKNESS = 8;
    static constexpr int16_t START_ANGLE = 135;
    static constexpr int16_t END_ANGLE = 45;
    static constexpr float ARC_SWEEP_DEGREES = 270.0f;
    static constexpr uint8_t CENTER_CIRCLE_SIZE = 14;
    static constexpr uint8_t INNER_CIRCLE_SIZE = 6;

    void createUI();
    void createArc();
    void createIndicator();
    void createCenterCircles();
    void setupDragInteraction();
    void applyColors();
    void updateArc();
    void updateIndicatorLine(float angleRad);
    void triggerFlash();
    static void flashTimerCallback(lv_timer_t* timer);
    static void dragEventCallback(lv_event_t* e);
    float normalizedToAngle(float normalized) const;
    void cleanup();

    // LVGL objects
    lv_obj_t* container_ = nullptr;
    lv_obj_t* arc_ = nullptr;
    lv_obj_t* indicator_ = nullptr;
    lv_obj_t* center_circle_ = nullptr;
    lv_obj_t* inner_circle_ = nullptr;
    lv_timer_t* flash_timer_ = nullptr;

    // Indicator line points
    lv_point_precise_t line_points_[2];

    // Configuration
    uint16_t width_ = DEFAULT_SIZE;
    uint16_t height_ = DEFAULT_SIZE;
    uint8_t color_index_ = 255;  // 255 = no macro color
    uint32_t track_color_ = 0;
    uint32_t value_color_ = 0;

    // State
    float value_ = 0.0f;
    float origin_ = 0.0f;
    bool centered_ = false;
    lv_coord_t drag_start_y_ = 0;

    // Cached geometry
    lv_coord_t arc_center_x_ = 0;
    lv_coord_t arc_center_y_ = 0;
};

}  // namespace oc::ui::lvgl
