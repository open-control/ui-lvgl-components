#pragma once

#include <algorithm>

#include <lvgl.h>

namespace oc::ui::lvgl {

/**
 * @brief Sizing mode for square widgets
 */
enum class SizeMode {
    Auto,             ///< Detect from container style (default)
    FitContent,       ///< Don't modify container, internal elements adapt to min(w,h)
    SquareFromWidth,  ///< Set container height = width
    SquareFromHeight, ///< Set container width = height
    Custom            ///< Return dimensions as-is, widget handles sizing
};

/**
 * @brief Size policy for widgets that need to maintain square aspect ratio
 *
 * This utility computes the appropriate size for square widgets based on
 * their container dimensions and the selected sizing mode.
 *
 * Usage in widgets:
 * @code
 * SquareSizePolicy size_policy_;
 *
 * void updateGeometry() {
 *     auto result = size_policy_.compute(container_);
 *     if (!result.valid) return;
 *
 *     if (result.modify_width) lv_obj_set_width(container_, result.width);
 *     if (result.modify_height) lv_obj_set_height(container_, result.height);
 *
 *     float size = std::min(result.width, result.height);
 *     // ... use size for internal elements
 * }
 * @endcode
 *
 * Direct usage on LVGL objects:
 * @code
 * SquareSizePolicy policy{SizeMode::SquareFromWidth};
 * auto result = policy.compute(some_lv_obj);
 * @endcode
 */
struct SquareSizePolicy {
    SizeMode mode = SizeMode::Auto;

    struct Result {
        lv_coord_t width;        ///< Computed width
        lv_coord_t height;       ///< Computed height
        bool modify_width;       ///< Should container width be modified?
        bool modify_height;      ///< Should container height be modified?
        bool valid;              ///< Is the result usable?
    };

    /**
     * @brief Compute the size based on container and mode
     * @param container The LVGL object to compute size for
     * @return Result with computed dimensions and modification flags
     */
    Result compute(lv_obj_t* container) const {
        if (!container) {
            return {0, 0, false, false, false};
        }

        lv_obj_update_layout(container);

        lv_coord_t w = lv_obj_get_width(container);
        lv_coord_t h = lv_obj_get_height(container);

        SizeMode effective = mode;

        if (mode == SizeMode::Auto) {
            effective = detectMode(container, w, h);
        }

        switch (effective) {
            case SizeMode::SquareFromWidth:
                return {w, w, false, true, w > 0};

            case SizeMode::SquareFromHeight:
                return {h, h, true, false, h > 0};

            case SizeMode::FitContent:
                return {w, h, false, false, w > 0 && h > 0};

            case SizeMode::Custom:
            default:
                return {w, h, false, false, w > 0 || h > 0};
        }
    }

private:
    /**
     * @brief Detect the appropriate mode from container style
     */
    static SizeMode detectMode(lv_obj_t* container, lv_coord_t w, lv_coord_t h) {
        lv_coord_t w_style = lv_obj_get_style_width(container, LV_PART_MAIN);
        lv_coord_t h_style = lv_obj_get_style_height(container, LV_PART_MAIN);

        bool w_is_content = (w_style == LV_SIZE_CONTENT);
        bool h_is_content = (h_style == LV_SIZE_CONTENT);

        if (h_is_content && !w_is_content && w > 0) {
            return SizeMode::SquareFromWidth;
        }
        if (w_is_content && !h_is_content && h > 0) {
            return SizeMode::SquareFromHeight;
        }

        return SizeMode::FitContent;
    }
};

}  // namespace oc::ui::lvgl
