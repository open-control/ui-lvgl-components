#pragma once

#include <SDL.h>
#include <functional>
#include <vector>
#include <string>

// Hardware IDs matching the real device
namespace HwId {
    // Buttons
    constexpr int LEFT_TOP = 10;
    constexpr int LEFT_CENTER = 11;
    constexpr int LEFT_BOTTOM = 12;
    constexpr int BOTTOM_LEFT = 20;
    constexpr int BOTTOM_CENTER = 21;
    constexpr int BOTTOM_RIGHT = 22;
    constexpr int NAV_BTN = 40;
    constexpr int MACRO_BTN_1 = 31;
    constexpr int MACRO_BTN_2 = 32;
    constexpr int MACRO_BTN_3 = 33;
    constexpr int MACRO_BTN_4 = 34;
    constexpr int MACRO_BTN_5 = 35;
    constexpr int MACRO_BTN_6 = 36;
    constexpr int MACRO_BTN_7 = 37;
    constexpr int MACRO_BTN_8 = 38;

    // Encoders
    constexpr int NAV_ENC = 400;
    constexpr int OPT_ENC = 410;
    constexpr int MACRO_ENC_1 = 301;
    constexpr int MACRO_ENC_2 = 302;
    constexpr int MACRO_ENC_3 = 303;
    constexpr int MACRO_ENC_4 = 304;
    constexpr int MACRO_ENC_5 = 305;
    constexpr int MACRO_ENC_6 = 306;
    constexpr int MACRO_ENC_7 = 307;
    constexpr int MACRO_ENC_8 = 308;
}

// ============================================================================
// HARDWARE LAYOUT - Derived from real controller measurements
// All dimensions derived from base ratios measured on physical hardware
// ============================================================================
namespace HwLayout {
    // ========================================================================
    // BASE REFERENCE VALUES (from real hardware measurements)
    // ========================================================================

    // Physical panel size (mm)
    constexpr float PANEL_MM = 190.0f;

    // Screen percentage of panel (measured from real hardware)
    constexpr float SCREEN_WIDTH_RATIO = 0.315789f;   // 31.5789% of panel width
    constexpr float SCREEN_HEIGHT_RATIO = 0.236842f;  // 23.6842% of panel height

    // Screen pixel size (fixed reference - actual display resolution)
    constexpr int SCREEN_W = 320;
    constexpr int SCREEN_H = 240;

    // ========================================================================
    // DERIVED PANEL SIZE & SCALE
    // ========================================================================

    // Panel size in pixels (derived from screen size and ratio)
    constexpr int PANEL_SIZE = static_cast<int>(SCREEN_W / SCREEN_WIDTH_RATIO);  // ~1013

    // Scale factor: pixels per mm
    constexpr float PX_PER_MM = PANEL_SIZE / PANEL_MM;  // ~5.33

    // ========================================================================
    // COMPONENT SIZES (physical dimensions in mm)
    // ========================================================================

    constexpr float BTN_SIZE_MM = 6.0f;      // Standard button diameter
    constexpr float NAV_SIZE_MM = 4.0f;      // NAV encoder diameter
    constexpr float OPT_SIZE_MM = 12.75f;    // Optical encoder diameter
    constexpr float MACRO_SIZE_MM = 7.3f;    // Macro encoder diameter

    // Derived pixel radii
    constexpr int BTN_RADIUS = static_cast<int>(BTN_SIZE_MM * PX_PER_MM);
    constexpr int NAV_RADIUS = static_cast<int>(NAV_SIZE_MM * PX_PER_MM);
    constexpr int OPT_RADIUS = static_cast<int>(OPT_SIZE_MM * PX_PER_MM);
    constexpr int MACRO_RADIUS = static_cast<int>(MACRO_SIZE_MM * PX_PER_MM);

    // ========================================================================
    // POSITION RATIOS (measured from real hardware layout)
    // ========================================================================

    // Screen position
    constexpr float SCREEN_TOP_RATIO = 0.079f;        // Screen top edge from panel top

    // Horizontal gaps from screen edge (in mm)
    constexpr float LEFT_GAP_MM = 30.0f;              // Gap between left buttons and screen
    constexpr float RIGHT_GAP_MM = 18.8f;             // Gap between right controls and screen

    // Vertical positions as ratio of panel height
    constexpr float BOTTOM_BTN_Y_RATIO = 0.395f;      // Bottom buttons row
    constexpr float MACRO_START_Y_RATIO = 0.612f;     // Macro grid top row

    // Macro grid spacing as ratio of panel size
    constexpr float MACRO_SPACING_X_RATIO = 0.219f;   // Horizontal spacing between macros
    constexpr float MACRO_SPACING_Y_RATIO = 0.197f;   // Vertical spacing between macro rows

    // ========================================================================
    // DERIVED POSITIONS
    // ========================================================================

    // Screen position (centered horizontally)
    constexpr int SCREEN_X = (PANEL_SIZE - SCREEN_W) / 2;
    constexpr int SCREEN_Y = static_cast<int>(PANEL_SIZE * SCREEN_TOP_RATIO);

    // Left buttons column - bounding box aligned with screen top/bottom
    constexpr int LEFT_BTN_X = SCREEN_X - static_cast<int>(LEFT_GAP_MM * PX_PER_MM);
    constexpr int LEFT_BTN_Y_TOP = SCREEN_Y + BTN_RADIUS;
    constexpr int LEFT_BTN_Y_CENTER = SCREEN_Y + SCREEN_H / 2;
    constexpr int LEFT_BTN_Y_BOTTOM = SCREEN_Y + SCREEN_H - BTN_RADIUS;

    // Right controls - bounding box aligned with screen top/bottom
    constexpr int RIGHT_X = SCREEN_X + SCREEN_W + static_cast<int>(RIGHT_GAP_MM * PX_PER_MM) + OPT_RADIUS;
    constexpr int NAV_Y = SCREEN_Y + NAV_RADIUS;
    constexpr int OPT_Y = SCREEN_Y + SCREEN_H - OPT_RADIUS;

    // Bottom buttons - bounding box aligned with screen left/right
    constexpr int BOTTOM_BTN_Y = static_cast<int>(PANEL_SIZE * BOTTOM_BTN_Y_RATIO);
    constexpr int BOTTOM_BTN_X_LEFT = SCREEN_X + BTN_RADIUS;
    constexpr int BOTTOM_BTN_X_CENTER = SCREEN_X + SCREEN_W / 2;
    constexpr int BOTTOM_BTN_X_RIGHT = SCREEN_X + SCREEN_W - BTN_RADIUS;

    // Macro encoders (4x2 grid) - centered horizontally
    constexpr int MACRO_SPACING_X = static_cast<int>(PANEL_SIZE * MACRO_SPACING_X_RATIO);
    constexpr int MACRO_SPACING_Y = static_cast<int>(PANEL_SIZE * MACRO_SPACING_Y_RATIO);
    constexpr int MACRO_START_X = (PANEL_SIZE - 3 * MACRO_SPACING_X) / 2;  // Centered
    constexpr int MACRO_START_Y = static_cast<int>(PANEL_SIZE * MACRO_START_Y_RATIO);

}

// Colors (matching the PDF)
namespace HwColor {
    constexpr uint32_t BACKGROUND = 0x3D2B1F;  // Dark wood brown
    constexpr uint32_t PANEL_BORDER = 0x2A1A10;
    constexpr uint32_t SCREEN_BG = 0x1A1A1A;

    // Left buttons (red gradient)
    constexpr uint32_t LEFT_TOP = 0xE53935;
    constexpr uint32_t LEFT_CENTER = 0xEF9A9A;
    constexpr uint32_t LEFT_BOTTOM = 0xFFCDD2;

    // Bottom buttons (green gradient)
    constexpr uint32_t BOTTOM_LEFT = 0x43A047;
    constexpr uint32_t BOTTOM_CENTER = 0x81C784;
    constexpr uint32_t BOTTOM_RIGHT = 0xC8E6C9;

    // Right controls
    constexpr uint32_t NAV = 0x1E88E5;
    constexpr uint32_t OPT = 0xAD1457;

    // Macro colors (orange to yellow gradient)
    constexpr uint32_t MACRO_1 = 0xEF6C00;
    constexpr uint32_t MACRO_2 = 0xF57C00;
    constexpr uint32_t MACRO_3 = 0xFDD835;
    constexpr uint32_t MACRO_4 = 0xC0CA33;
    constexpr uint32_t MACRO_5 = 0xFFCC80;
    constexpr uint32_t MACRO_6 = 0xFFE082;
    constexpr uint32_t MACRO_7 = 0xFFF59D;
    constexpr uint32_t MACRO_8 = 0xF0F4C3;
}

// Callbacks
using ButtonCallback = std::function<void(int id, bool pressed)>;
using EncoderCallback = std::function<void(int id, float delta)>;

// Hardware button (simple circle)
struct HwButton {
    int id;
    int x, y, radius;
    uint32_t color;
    bool pressed = false;

    bool hitTest(int mx, int my) const;
    void render(SDL_Renderer* renderer) const;
};

// Hardware encoder with optional button
struct HwEncoder {
    int encId;
    int btnId;          // -1 if no button
    int x, y, radius;
    uint32_t color;
    float value = 0.5f; // 0.0 - 1.0 for absolute, delta for relative
    bool isRelative = false;
    bool pressed = false;
    bool dragging = false;
    int dragStartY = 0;

    bool hitTest(int mx, int my) const;
    void render(SDL_Renderer* renderer) const;
};

// Control info for legend display
struct ControlInfo {
    enum class Type { NONE, BUTTON, ENCODER };
    Type type = Type::NONE;
    int id = -1;
    float value = 0.0f;
    bool pressed = false;
    const char* name = "";
};

// Main hardware simulator
class HwSimulator {
public:
    HwSimulator();

    void init(SDL_Renderer* renderer);
    void render();
    void handleEvent(const SDL_Event& event);

    // Callbacks
    void setButtonCallback(ButtonCallback cb) { buttonCallback_ = cb; }
    void setEncoderCallback(EncoderCallback cb) { encoderCallback_ = cb; }

    // Get LVGL screen rect
    SDL_Rect getScreenRect() const {
        return {HwLayout::SCREEN_X, HwLayout::SCREEN_Y,
                HwLayout::SCREEN_W, HwLayout::SCREEN_H};
    }

    // Get panel size
    int getPanelSize() const { return HwLayout::PANEL_SIZE; }

    // Set encoder value externally (for sync with LVGL)
    void setEncoderValue(int encId, float value);

private:
    SDL_Renderer* renderer_ = nullptr;
    std::vector<HwButton> buttons_;
    std::vector<HwEncoder> encoders_;

    ButtonCallback buttonCallback_;
    EncoderCallback encoderCallback_;

    HwEncoder* activeEncoder_ = nullptr;

    // Active control tracking for legend
    ControlInfo activeControl_;
    uint32_t activeControlTime_ = 0;

    void setupControls();
    void renderPanel();
    void renderLegend();
    const char* getControlName(int id) const;
};
