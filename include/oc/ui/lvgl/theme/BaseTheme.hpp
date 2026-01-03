#pragma once

#include <cstdint>

namespace oc::ui::lvgl {

/**
 * @brief Base theme constants for LVGL components
 *
 * Provides default colors, layout values, and animation timing.
 * Widgets use these as defaults but allow override via fluent setters.
 */
namespace BaseTheme {

namespace Color {

// Macro colors (for parameter widgets, 8-color palette)
constexpr uint32_t MACRO_1_RED = 0xF41B3E;
constexpr uint32_t MACRO_2_ORANGE = 0xFF7F17;
constexpr uint32_t MACRO_3_YELLOW = 0xFCEB23;
constexpr uint32_t MACRO_4_GREEN = 0x5BC515;
constexpr uint32_t MACRO_5_CYAN = 0x65CE92;
constexpr uint32_t MACRO_6_BLUE = 0x5CA6EE;
constexpr uint32_t MACRO_7_PURPLE = 0xC36EFF;
constexpr uint32_t MACRO_8_PINK = 0xFF54B0;

constexpr uint32_t MACROS[8] = {
    MACRO_1_RED,  MACRO_2_ORANGE, MACRO_3_YELLOW, MACRO_4_GREEN,
    MACRO_5_CYAN, MACRO_6_BLUE,   MACRO_7_PURPLE, MACRO_8_PINK
};

// UI colors
constexpr uint32_t BACKGROUND = 0x000000;
constexpr uint32_t INACTIVE = 0x333333;
constexpr uint32_t INACTIVE_LIGHTER = 0x666666;
constexpr uint32_t ACTIVE = 0xECA747;
constexpr uint32_t TEXT_PRIMARY = 0xFFFFFF;
constexpr uint32_t TEXT_PRIMARY_INVERTED = 0x292929;
constexpr uint32_t TEXT_SECONDARY = 0xD9D9D9;

// Status colors
constexpr uint32_t STATUS_SUCCESS = 0x00FF00;
constexpr uint32_t STATUS_WARNING = 0xFFA500;
constexpr uint32_t STATUS_ERROR = 0xFF0000;
constexpr uint32_t STATUS_INACTIVE = 0x606060;

// Knob widget colors
constexpr uint32_t KNOB_BACKGROUND = 0x202020;
constexpr uint32_t KNOB_VALUE = 0x909090;
constexpr uint32_t KNOB_TRACK = 0x606060;

/**
 * @brief Get macro color by index (0-7)
 * @param index Color index, clamped to valid range
 * @return Color value
 */
inline uint32_t getMacroColor(uint8_t index) {
    return (index < 8) ? MACROS[index] : INACTIVE;
}

}  // namespace Color

namespace Layout {

// Spacing scale (base unit: 2px)
constexpr int16_t SPACE_XS = 2;
constexpr int16_t SPACE_SM = 4;
constexpr int16_t SPACE_MD = 8;
constexpr int16_t SPACE_LG = 12;
constexpr int16_t SPACE_XL = 16;

// Margins (aliases for semantic use)
constexpr int16_t MARGIN_XS = SPACE_XS;
constexpr int16_t MARGIN_SM = SPACE_SM;
constexpr int16_t MARGIN_MD = SPACE_MD;
constexpr int16_t MARGIN_LG = SPACE_XL;

// Button padding
constexpr int16_t PAD_BUTTON_H = 8;
constexpr int16_t PAD_BUTTON_V = 6;

// List specific
constexpr int16_t LIST_ITEM_GAP = 2;
constexpr int16_t LIST_PAD = 4;
constexpr int16_t SCROLLBAR_WIDTH = 3;

// Row gaps
constexpr int16_t ROW_GAP_SM = 2;
constexpr int16_t ROW_GAP_MD = 4;

}  // namespace Layout

// =============================================================================
// Opacity - Semantic opacity levels
// =============================================================================
namespace Opacity {

constexpr uint8_t OPA_FULL = 255;      // LV_OPA_COVER - fully visible
constexpr uint8_t OPA_90 = 230;        // ~90% - modal backgrounds, overlays
constexpr uint8_t OPA_70 = 178;        // ~70% - secondary elements, hints
constexpr uint8_t OPA_50 = 127;        // ~50% - disabled, dimmed
constexpr uint8_t OPA_30 = 77;         // ~30% - scrollbars, subtle hints
constexpr uint8_t OPA_TRANSP = 0;      // LV_OPA_TRANSP - invisible

}  // namespace Opacity

// =============================================================================
// Animation - Timing constants
// =============================================================================
namespace Animation {

// Standard duration scale
constexpr uint32_t INSTANT_MS = 0;
constexpr uint32_t FAST_MS = 100;
constexpr uint32_t NORMAL_MS = 300;
constexpr uint32_t SLOW_MS = 500;

// Component-specific (scroll, flash)
constexpr uint32_t SCROLL_ANIM_MS = 50;
constexpr uint32_t SCROLL_START_DELAY_MS = 500;
constexpr uint32_t OVERFLOW_CHECK_DELAY_MS = 50;
constexpr uint32_t FLASH_DURATION_MS = FAST_MS;

}  // namespace Animation

}  // namespace BaseTheme

}  // namespace oc::ui::lvgl
