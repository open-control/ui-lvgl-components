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
 * - Grid layout support via gridCell() helper
 * - Full LVGL compatibility
 *
 * ## Basic Usage
 * @code
 * Label label(parent);
 * label.setText("This is a very long text that will scroll");
 * @endcode
 *
 * ## Grid Layout Usage (IMPORTANT)
 *
 * This widget uses LV_PCT(100) internally, which has a subtle interaction with
 * LVGL grids: LV_PCT(100) refers to the PARENT container, not the grid cell.
 * This means LV_GRID_ALIGN_START/CENTER/END won't position the Label within
 * its cell - it will already span the full parent width.
 *
 * **Solution**: Always use gridCell() which applies LV_GRID_ALIGN_STRETCH,
 * then use alignment() for text positioning within the cell.
 *
 * @code
 * // CORRECT: Use gridCell() + alignment()
 * label->alignment(LV_TEXT_ALIGN_RIGHT)  // Text aligned right within cell
 *      .gridCell(2, 1, 0, 1)             // Column 2, row 0
 *      .color(0xFFFFFF);
 *
 * // WRONG: Don't use lv_obj_set_grid_cell with GRID_ALIGN_END
 * // This won't work because LV_PCT(100) makes the container full-width
 * lv_obj_set_grid_cell(label, LV_GRID_ALIGN_END, 2, 1, ...);  // DON'T DO THIS
 * @endcode
 *
 * ## Flex Layout Usage
 * @code
 * label.flexGrow(true);  // Expands to fill available space in flex container
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
    Label& autoScroll(bool enabled);

    /**
     * @brief Set text alignment within the container
     *
     * Controls where text appears when it fits within the container width.
     * When text overflows and auto-scroll is enabled, text starts left-aligned
     * for the scroll animation.
     *
     * In grid layouts, use this instead of LV_GRID_ALIGN_START/CENTER/END
     * (see class documentation for details).
     *
     * @param align LV_TEXT_ALIGN_LEFT, LV_TEXT_ALIGN_CENTER, or LV_TEXT_ALIGN_RIGHT
     */
    Label& alignment(lv_text_align_t align);

    /** @brief Enable flex-grow for layout */
    Label& flexGrow(bool enabled);

    /** @brief Set text color */
    Label& color(uint32_t c);

    /** @brief Set font */
    Label& font(const lv_font_t* f);

    /** @brief Set fixed width (disables flex-grow) */
    Label& width(lv_coord_t w);

    /**
     * @brief Control LVGL object ownership
     *
     * When true (default): Label destructor deletes container/label LVGL objects.
     * When false: Label does NOT delete - LVGL parent-child hierarchy handles cleanup.
     *
     * Use false when Label is embedded in an LVGL tree that will be bulk-deleted
     * (e.g., via lv_obj_clean() or lv_obj_delete() on parent).
     */
    Label& ownsLvglObjects(bool owns);

    /**
     * @brief Place Label in a grid cell (REQUIRED for grid layouts)
     *
     * This method MUST be used instead of lv_obj_set_grid_cell() when placing
     * a Label in a grid layout. It applies LV_GRID_ALIGN_STRETCH horizontally,
     * which is required because the Label container uses LV_PCT(100) width.
     *
     * For horizontal text positioning, use alignment() with:
     * - LV_TEXT_ALIGN_LEFT (default left alignment)
     * - LV_TEXT_ALIGN_CENTER (centered text)
     * - LV_TEXT_ALIGN_RIGHT (right-aligned text)
     *
     * @code
     * // Right-aligned text in column 2, row 0
     * label->alignment(LV_TEXT_ALIGN_RIGHT)
     *      .gridCell(2, 1, 0, 1);
     *
     * // Centered text spanning 2 columns
     * label->alignment(LV_TEXT_ALIGN_CENTER)
     *      .gridCell(0, 2, 0, 1);
     * @endcode
     *
     * @param col Column index (0-based)
     * @param colSpan Number of columns to span
     * @param row Row index (0-based)
     * @param rowSpan Number of rows to span
     * @param vAlign Vertical alignment within cell (default: LV_GRID_ALIGN_CENTER)
     */
    Label& gridCell(uint8_t col, uint8_t colSpan, uint8_t row, uint8_t rowSpan,
                    lv_grid_align_t vAlign = LV_GRID_ALIGN_CENTER);

    // =========================================================================
    // Data Setters
    // =========================================================================

    /** @brief Set label text */
    void setText(const std::string& text);
    void setText(const char* text);

    /**
     * @brief Set label text from integer
     * @param value The integer value to display
     * @param prefix Optional prefix string (e.g., "$")
     * @param suffix Optional suffix string (e.g., " items")
     */
    void setText(int value, const char* prefix = "", const char* suffix = "");

    /**
     * @brief Set label text from float
     * @param value The float value to display
     * @param decimals Number of decimal places (default: 2)
     * @param prefix Optional prefix string
     * @param suffix Optional suffix string (e.g., " BPM", " %")
     */
    void setText(float value, uint8_t decimals = 2, const char* prefix = "", const char* suffix = "");

private:
    void createWidgets(lv_obj_t* parent);
    void cleanup();

    void checkOverflowAndScroll();
    void applyStaticAlignment();
    void startScrollAnimation();
    void stopScrollAnimation();

    static void scrollAnimCallback(void* var, int32_t value);
    static void pauseTimerCallback(lv_timer_t* timer);
    static void sizeChangedCallback(lv_event_t* e);

    lv_obj_t* container_ = nullptr;
    lv_obj_t* label_ = nullptr;
    lv_anim_t scroll_anim_;
    lv_timer_t* pending_timer_ = nullptr;

    bool auto_scroll_enabled_ = true;
    bool anim_running_ = false;
    bool owns_lvgl_objects_ = true;
    lv_coord_t overflow_amount_ = 0;
    lv_text_align_t alignment_ = LV_TEXT_ALIGN_CENTER;

    uint32_t scroll_duration_ms_ = 2000;
    uint32_t pause_duration_ms_ = 1000;
};

}  // namespace oc::ui::lvgl
