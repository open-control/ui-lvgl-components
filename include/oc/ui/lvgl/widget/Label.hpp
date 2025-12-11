#pragma once

#include <string>

#include <lvgl.h>

#include <oc/ui/lvgl/IWidget.hpp>

#include "../theme/BaseTheme.hpp"

namespace oc::ui::lvgl {

/**
 * @brief Label widget with optional auto-scroll for overflow text
 *
 * Features:
 * - Optional auto-scroll animation when text exceeds container width
 * - Configurable scroll timing and delays
 * - Flex-grow support for layout integration
 * - Full LVGL compatibility
 *
 * Usage:
 * @code
 * Label label(parent);
 * label.setText("This is a very long text that will scroll");
 *
 * // Fluent configuration
 * Label label = Label(parent)
 *     .autoScroll(true)
 *     .alignment(LV_TEXT_ALIGN_CENTER);
 *
 * // Direct LVGL access
 * lv_obj_set_style_text_color(label, lv_color_hex(0xFF0000), 0);
 * @endcode
 */
class Label : public IWidget {
public:
    // =========================================================================
    // Construction / Destruction
    // =========================================================================

    explicit Label(lv_obj_t* parent);
    ~Label();

    // Move only
    Label(Label&& other) noexcept;
    Label& operator=(Label&& other) noexcept;
    Label(const Label&) = delete;
    Label& operator=(const Label&) = delete;

    // =========================================================================
    // LVGL Access
    // =========================================================================

    /** @brief Get container element */
    lv_obj_t* getElement() const override { return container_; }

    /** @brief Implicit conversion for LVGL functions */
    operator lv_obj_t*() const { return container_; }

    /** @brief Get the actual label element */
    lv_obj_t* getLabel() const { return label_; }

    // =========================================================================
    // Fluent Configuration
    // =========================================================================

    /** @brief Enable/disable auto-scroll */
    Label& autoScroll(bool enabled) &;
    Label autoScroll(bool enabled) &&;

    /** @brief Set text alignment (when not scrolling) */
    Label& alignment(lv_text_align_t align) &;
    Label alignment(lv_text_align_t align) &&;

    /** @brief Enable flex-grow for layout */
    Label& flexGrow(bool enabled) &;
    Label flexGrow(bool enabled) &&;

    /** @brief Set text color */
    Label& color(uint32_t c) &;
    Label color(uint32_t c) &&;

    /** @brief Set font */
    Label& font(const lv_font_t* f) &;
    Label font(const lv_font_t* f) &&;

    /** @brief Set fixed width (disables flex-grow, enables auto-scroll) */
    Label& width(lv_coord_t w) &;
    Label width(lv_coord_t w) &&;

    // =========================================================================
    // Data Setters
    // =========================================================================

    /** @brief Set label text */
    void setText(const std::string& text);
    void setText(const char* text);

private:
    void createWidgets(lv_obj_t* parent);
    void cleanup();

    void checkOverflowAndScroll();
    void startScrollAnimation();
    void stopScrollAnimation();

    static void scrollAnimCallback(void* var, int32_t value);
    static void pauseTimerCallback(lv_timer_t* timer);

    lv_obj_t* container_ = nullptr;
    lv_obj_t* label_ = nullptr;
    lv_anim_t scroll_anim_;

    bool auto_scroll_enabled_ = true;
    bool anim_running_ = false;
    lv_coord_t overflow_amount_ = 0;
    lv_text_align_t alignment_ = LV_TEXT_ALIGN_LEFT;

    uint32_t scroll_duration_ms_ = 2000;
    uint32_t pause_duration_ms_ = 1000;
};

}  // namespace oc::ui::lvgl
