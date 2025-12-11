#pragma once

#include <cstdint>

#include <lvgl.h>

#include <oc/ui/lvgl/IWidget.hpp>

#include "../theme/BaseTheme.hpp"

namespace oc::ui::lvgl {

/**
 * @brief Widget for displaying enumerated/discrete values
 *
 * Provides a container with a horizontal indicator line at the top
 * that can flash on value changes. Content is added to inner().
 *
 * Usage:
 * @code
 * EnumWidget widget(parent);
 * widget.lineColor(0x606060)
 *       .flashColor(0xECA747);
 *
 * lv_obj_t* label = lv_label_create(widget.inner());
 * lv_label_set_text(label, "Sawtooth");
 *
 * widget.triggerFlash();  // Flash on value change
 * @endcode
 */
class EnumWidget : public IWidget {
public:
    explicit EnumWidget(lv_obj_t* parent);
    ~EnumWidget();

    // Move only
    EnumWidget(EnumWidget&& other) noexcept;
    EnumWidget& operator=(EnumWidget&& other) noexcept;
    EnumWidget(const EnumWidget&) = delete;
    EnumWidget& operator=(const EnumWidget&) = delete;

    // LVGL Access
    lv_obj_t* getElement() const override { return container_; }
    operator lv_obj_t*() const { return container_; }

    /** @brief Parent object for adding content */
    lv_obj_t* inner() const { return inner_; }

    // Fluent Configuration
    /** @brief Background color of container */
    EnumWidget& bgColor(uint32_t color) &;
    EnumWidget bgColor(uint32_t color) &&;

    /** @brief Color of the indicator line */
    EnumWidget& lineColor(uint32_t color) &;
    EnumWidget lineColor(uint32_t color) &&;

    /** @brief Flash color for indicator line */
    EnumWidget& flashColor(uint32_t color) &;
    EnumWidget flashColor(uint32_t color) &&;

    // Actions
    /** @brief Trigger a flash animation on the indicator line */
    void triggerFlash();

private:
    static constexpr uint16_t DEFAULT_WIDTH = 62;
    static constexpr lv_coord_t LINE_HEIGHT = 2;
    static constexpr lv_coord_t LINE_MARGIN = 4;
    static constexpr lv_coord_t LINE_TOP_MARGIN = 2;
    static constexpr lv_coord_t LINE_BOTTOM_MARGIN = 2;
    static constexpr lv_coord_t INNER_HEIGHT = 40;

    void createUI();
    void cleanup();
    void applyColors();
    static void flashTimerCallback(lv_timer_t* timer);

    // LVGL objects
    lv_obj_t* container_ = nullptr;
    lv_obj_t* inner_ = nullptr;
    lv_obj_t* top_line_ = nullptr;
    lv_timer_t* flash_timer_ = nullptr;

    // Configuration
    uint32_t bg_color_ = 0;
    uint32_t line_color_ = 0;
    uint32_t flash_color_ = 0;
};

}  // namespace oc::ui::lvgl
