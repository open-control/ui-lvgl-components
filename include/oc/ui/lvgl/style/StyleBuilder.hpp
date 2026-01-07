#pragma once

#include <oc/ui/lvgl/theme/BaseTheme.hpp>

#include <lvgl.h>

namespace oc::ui::lvgl::style {

/**
 * @brief Fluent API for applying LVGL styles
 *
 * Provides a chainable interface for common style patterns.
 * Uses BaseTheme defaults but allows override via parameters.
 *
 * ## Usage
 * @code
 * // Apply transparent container with flex row layout
 * style::apply(container).transparent().flexRow().noScroll();
 *
 * // With custom gap
 * style::apply(panel).transparent().flexColumn(LV_FLEX_ALIGN_START, 8);
 * @endcode
 */
class StyleBuilder {
public:
    /// Construct builder for target object
    explicit StyleBuilder(lv_obj_t* obj) : obj_(obj) {}

    // =========================================================================
    // Background & Border
    // =========================================================================

    /**
     * @brief Apply transparent container style
     *
     * Sets: transparent background, no border, no padding.
     * Common pattern for layout containers.
     */
    StyleBuilder& transparent() {
        lv_obj_set_style_bg_opa(obj_, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(obj_, 0, 0);
        lv_obj_set_style_pad_all(obj_, 0, 0);
        return *this;
    }

    /**
     * @brief Set background color
     * @param color Hex color (e.g., 0xFF0000 for red)
     * @param opa Opacity (default: fully opaque)
     */
    StyleBuilder& bgColor(uint32_t color, lv_opa_t opa = LV_OPA_COVER) {
        lv_obj_set_style_bg_color(obj_, lv_color_hex(color), 0);
        lv_obj_set_style_bg_opa(obj_, opa, 0);
        return *this;
    }

    /**
     * @brief Set text color
     * @param color Hex color (e.g., 0xFFFFFF for white)
     */
    StyleBuilder& textColor(uint32_t color) {
        lv_obj_set_style_text_color(obj_, lv_color_hex(color), 0);
        return *this;
    }

    /**
     * @brief Remove border
     */
    StyleBuilder& noBorder() {
        lv_obj_set_style_border_width(obj_, 0, 0);
        return *this;
    }

    /**
     * @brief Set border width and color
     */
    StyleBuilder& border(int16_t width, uint32_t color) {
        lv_obj_set_style_border_width(obj_, width, 0);
        lv_obj_set_style_border_color(obj_, lv_color_hex(color), 0);
        return *this;
    }

    /**
     * @brief Set corner radius
     */
    StyleBuilder& radius(int16_t r) {
        lv_obj_set_style_radius(obj_, r, 0);
        return *this;
    }

    // =========================================================================
    // Flex Layout
    // =========================================================================

    /**
     * @brief Apply flex row layout
     * @param hAlign Horizontal alignment (default: START)
     * @param gap Gap between items (default: ROW_GAP_MD from BaseTheme)
     */
    StyleBuilder& flexRow(lv_flex_align_t hAlign = LV_FLEX_ALIGN_START,
                          int16_t gap = base_theme::layout::ROW_GAP_MD) {
        lv_obj_set_flex_flow(obj_, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(obj_, hAlign, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_gap(obj_, gap, 0);
        return *this;
    }

    /**
     * @brief Apply flex column layout
     * @param vAlign Vertical alignment (default: START)
     * @param gap Gap between items (default: ROW_GAP_MD from BaseTheme)
     */
    StyleBuilder& flexColumn(lv_flex_align_t vAlign = LV_FLEX_ALIGN_START,
                             int16_t gap = base_theme::layout::ROW_GAP_MD) {
        lv_obj_set_flex_flow(obj_, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(obj_, LV_FLEX_ALIGN_CENTER, vAlign, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_gap(obj_, gap, 0);
        return *this;
    }

    // =========================================================================
    // Size
    // =========================================================================

    /**
     * @brief Set full size (100% x 100%)
     */
    StyleBuilder& fullSize() {
        lv_obj_set_size(obj_, LV_PCT(100), LV_PCT(100));
        return *this;
    }

    /**
     * @brief Set specific size
     */
    StyleBuilder& size(lv_coord_t width, lv_coord_t height) {
        lv_obj_set_size(obj_, width, height);
        return *this;
    }

    // =========================================================================
    // Padding
    // =========================================================================

    /**
     * @brief Set padding on all sides
     */
    StyleBuilder& pad(int16_t all) {
        lv_obj_set_style_pad_all(obj_, all, 0);
        return *this;
    }

    /**
     * @brief Set horizontal padding (left + right)
     */
    StyleBuilder& padH(int16_t h) {
        lv_obj_set_style_pad_left(obj_, h, 0);
        lv_obj_set_style_pad_right(obj_, h, 0);
        return *this;
    }

    /**
     * @brief Set vertical padding (top + bottom)
     */
    StyleBuilder& padV(int16_t v) {
        lv_obj_set_style_pad_top(obj_, v, 0);
        lv_obj_set_style_pad_bottom(obj_, v, 0);
        return *this;
    }

    /// Set left padding
    StyleBuilder& padLeft(int16_t v) {
        lv_obj_set_style_pad_left(obj_, v, 0);
        return *this;
    }

    /// Set right padding
    StyleBuilder& padRight(int16_t v) {
        lv_obj_set_style_pad_right(obj_, v, 0);
        return *this;
    }

    /// Set top padding
    StyleBuilder& padTop(int16_t v) {
        lv_obj_set_style_pad_top(obj_, v, 0);
        return *this;
    }

    /// Set bottom padding
    StyleBuilder& padBottom(int16_t v) {
        lv_obj_set_style_pad_bottom(obj_, v, 0);
        return *this;
    }

    /// Set row gap (vertical spacing between flex items)
    StyleBuilder& padRow(int16_t v) {
        lv_obj_set_style_pad_row(obj_, v, 0);
        return *this;
    }

    /// Set column gap (horizontal spacing between flex items)
    StyleBuilder& padColumn(int16_t v) {
        lv_obj_set_style_pad_column(obj_, v, 0);
        return *this;
    }

    // =========================================================================
    // Margin
    // =========================================================================

    /// Set margin on all sides
    StyleBuilder& margin(int16_t all) {
        lv_obj_set_style_margin_left(obj_, all, 0);
        lv_obj_set_style_margin_right(obj_, all, 0);
        lv_obj_set_style_margin_top(obj_, all, 0);
        lv_obj_set_style_margin_bottom(obj_, all, 0);
        return *this;
    }

    /// Set horizontal margin (left + right)
    StyleBuilder& marginH(int16_t h) {
        lv_obj_set_style_margin_left(obj_, h, 0);
        lv_obj_set_style_margin_right(obj_, h, 0);
        return *this;
    }

    /// Set vertical margin (top + bottom)
    StyleBuilder& marginV(int16_t v) {
        lv_obj_set_style_margin_top(obj_, v, 0);
        lv_obj_set_style_margin_bottom(obj_, v, 0);
        return *this;
    }

    /// Set left margin
    StyleBuilder& marginLeft(int16_t v) {
        lv_obj_set_style_margin_left(obj_, v, 0);
        return *this;
    }

    /// Set right margin
    StyleBuilder& marginRight(int16_t v) {
        lv_obj_set_style_margin_right(obj_, v, 0);
        return *this;
    }

    /// Set top margin
    StyleBuilder& marginTop(int16_t v) {
        lv_obj_set_style_margin_top(obj_, v, 0);
        return *this;
    }

    /// Set bottom margin
    StyleBuilder& marginBottom(int16_t v) {
        lv_obj_set_style_margin_bottom(obj_, v, 0);
        return *this;
    }

    // =========================================================================
    // Text
    // =========================================================================

    /// Set text font
    StyleBuilder& textFont(const lv_font_t* font) {
        lv_obj_set_style_text_font(obj_, font, 0);
        return *this;
    }

    /// Set text alignment
    StyleBuilder& textAlign(lv_text_align_t align) {
        lv_obj_set_style_text_align(obj_, align, 0);
        return *this;
    }

    /// Set text opacity
    StyleBuilder& textOpa(lv_opa_t opa) {
        lv_obj_set_style_text_opa(obj_, opa, 0);
        return *this;
    }

    // =========================================================================
    // Opacity
    // =========================================================================

    /// Set global opacity
    StyleBuilder& opa(lv_opa_t o) {
        lv_obj_set_style_opa(obj_, o, 0);
        return *this;
    }

    /// Set background opacity only
    StyleBuilder& bgOpa(lv_opa_t o) {
        lv_obj_set_style_bg_opa(obj_, o, 0);
        return *this;
    }

    // =========================================================================
    // Flags
    // =========================================================================

    /**
     * @brief Disable scrolling
     */
    StyleBuilder& noScroll() {
        lv_obj_clear_flag(obj_, LV_OBJ_FLAG_SCROLLABLE);
        return *this;
    }

    /**
     * @brief Set visibility
     */
    StyleBuilder& visible(bool isVisible) {
        if (isVisible) {
            lv_obj_clear_flag(obj_, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(obj_, LV_OBJ_FLAG_HIDDEN);
        }
        return *this;
    }

private:
    lv_obj_t* obj_;
};

// =============================================================================
// Entry Point
// =============================================================================

/**
 * @brief Create StyleBuilder for an object
 * @param obj LVGL object to style
 * @return StyleBuilder for fluent chaining
 *
 * @code
 * style::apply(container).transparent().flexRow().noScroll();
 * @endcode
 */
inline StyleBuilder apply(lv_obj_t* obj) {
    return StyleBuilder(obj);
}

}  // namespace oc::ui::lvgl::style
