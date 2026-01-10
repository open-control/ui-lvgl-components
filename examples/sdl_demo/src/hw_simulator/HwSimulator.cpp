#include "HwSimulator.hpp"
#include "SDL2_gfxPrimitives.h"
#include <cmath>
#include <algorithm>

// Helper to extract RGB from uint32_t color
static void colorToRGB(uint32_t color, Uint8& r, Uint8& g, Uint8& b) {
    r = (color >> 16) & 0xFF;
    g = (color >> 8) & 0xFF;
    b = color & 0xFF;
}

// ============================================================================
// HwButton
// ============================================================================

bool HwButton::hitTest(int mx, int my) const {
    int dx = mx - x;
    int dy = my - y;
    return (dx * dx + dy * dy) <= (radius * radius);
}

void HwButton::render(SDL_Renderer* renderer) const {
    Uint8 r, g, b;
    colorToRGB(color, r, g, b);

    // Darken if pressed
    if (pressed) {
        r = static_cast<Uint8>(r * 0.7f);
        g = static_cast<Uint8>(g * 0.7f);
        b = static_cast<Uint8>(b * 0.7f);
    }

    // Filled circle with anti-aliased edge
    filledCircleRGBA(renderer, x, y, radius, r, g, b, 255);
    aacircleRGBA(renderer, x, y, radius, r, g, b, 255);      // Same color AA edge
    aacircleRGBA(renderer, x, y, radius + 1, 40, 40, 40, 180); // Subtle dark outline
}

// ============================================================================
// HwEncoder
// ============================================================================

bool HwEncoder::hitTest(int mx, int my) const {
    int dx = mx - x;
    int dy = my - y;
    return (dx * dx + dy * dy) <= (radius * radius);
}

void HwEncoder::render(SDL_Renderer* renderer) const {
    Uint8 r, g, b;
    colorToRGB(color, r, g, b);

    // Background circle (darker) with AA
    Uint8 bgR = static_cast<Uint8>(r * 0.25f);
    Uint8 bgG = static_cast<Uint8>(g * 0.25f);
    Uint8 bgB = static_cast<Uint8>(b * 0.25f);
    filledCircleRGBA(renderer, x, y, radius, bgR, bgG, bgB, 255);
    aacircleRGBA(renderer, x, y, radius, bgR, bgG, bgB, 255);

    // Value arc (from 7 o'clock to current value) - thicker with AA
    if (!isRelative) {
        int startAngle = 135;
        int endAngle = 135 + static_cast<int>(value * 270);

        // Draw thick arc with multiple AA lines
        int arcThickness = 8;
        for (int thickness = 0; thickness < arcThickness; thickness++) {
            int currentRadius = radius - thickness;
            if (currentRadius < radius / 2) break;

            for (int angle = startAngle; angle < endAngle && angle < startAngle + 270; angle++) {
                float rad = angle * M_PI / 180.0f;
                float radNext = (angle + 1) * M_PI / 180.0f;
                int x1 = x + static_cast<int>(currentRadius * cosf(rad));
                int y1 = y + static_cast<int>(currentRadius * sinf(rad));
                int x2 = x + static_cast<int>(currentRadius * cosf(radNext));
                int y2 = y + static_cast<int>(currentRadius * sinf(radNext));
                aalineRGBA(renderer, x1, y1, x2, y2, r, g, b, 255);
            }
        }
    }

    // Center button (if has button) with AA
    if (btnId >= 0) {
        int centerRadius = radius / 3;
        Uint8 centerR = pressed ? static_cast<Uint8>(r * 0.5f) : r;
        Uint8 centerG = pressed ? static_cast<Uint8>(g * 0.5f) : g;
        Uint8 centerB = pressed ? static_cast<Uint8>(b * 0.5f) : b;
        filledCircleRGBA(renderer, x, y, centerRadius, centerR, centerG, centerB, 255);
        aacircleRGBA(renderer, x, y, centerRadius, centerR, centerG, centerB, 255);
        aacircleRGBA(renderer, x, y, centerRadius + 1, 40, 40, 40, 120);
    }

    // Outer ring with AA
    aacircleRGBA(renderer, x, y, radius, 60, 60, 60, 200);
    aacircleRGBA(renderer, x, y, radius + 1, 40, 40, 40, 150);
}

// ============================================================================
// HwSimulator
// ============================================================================

HwSimulator::HwSimulator() {
    setupControls();
}

void HwSimulator::init(SDL_Renderer* renderer) {
    renderer_ = renderer;
}

void HwSimulator::setupControls() {
    using namespace HwLayout;

    // Left buttons
    buttons_.push_back({HwId::LEFT_TOP, LEFT_BTN_X, LEFT_BTN_Y_TOP, BTN_RADIUS, HwColor::LEFT_TOP});
    buttons_.push_back({HwId::LEFT_CENTER, LEFT_BTN_X, LEFT_BTN_Y_CENTER, BTN_RADIUS, HwColor::LEFT_CENTER});
    buttons_.push_back({HwId::LEFT_BOTTOM, LEFT_BTN_X, LEFT_BTN_Y_BOTTOM, BTN_RADIUS, HwColor::LEFT_BOTTOM});

    // Bottom buttons
    buttons_.push_back({HwId::BOTTOM_LEFT, BOTTOM_BTN_X_LEFT, BOTTOM_BTN_Y, BTN_RADIUS, HwColor::BOTTOM_LEFT});
    buttons_.push_back({HwId::BOTTOM_CENTER, BOTTOM_BTN_X_CENTER, BOTTOM_BTN_Y, BTN_RADIUS, HwColor::BOTTOM_CENTER});
    buttons_.push_back({HwId::BOTTOM_RIGHT, BOTTOM_BTN_X_RIGHT, BOTTOM_BTN_Y, BTN_RADIUS, HwColor::BOTTOM_RIGHT});

    // NAV encoder (with button)
    encoders_.push_back({HwId::NAV_ENC, HwId::NAV_BTN, RIGHT_X, NAV_Y, NAV_RADIUS, HwColor::NAV, 0.5f, true});

    // OPT encoder (no button, relative)
    encoders_.push_back({HwId::OPT_ENC, -1, RIGHT_X, OPT_Y, OPT_RADIUS, HwColor::OPT, 0.0f, true});

    // Macro encoders (with buttons)
    const uint32_t macroColors[8] = {
        HwColor::MACRO_1, HwColor::MACRO_2, HwColor::MACRO_3, HwColor::MACRO_4,
        HwColor::MACRO_5, HwColor::MACRO_6, HwColor::MACRO_7, HwColor::MACRO_8
    };

    for (int i = 0; i < 8; i++) {
        int col = i % 4;
        int row = i / 4;
        int mx = MACRO_START_X + col * MACRO_SPACING_X;
        int my = MACRO_START_Y + row * MACRO_SPACING_Y;
        encoders_.push_back({
            HwId::MACRO_ENC_1 + i,
            HwId::MACRO_BTN_1 + i,
            mx, my, MACRO_RADIUS,
            macroColors[i],
            0.5f, false
        });
    }
}

void HwSimulator::renderPanel() {
    using namespace HwLayout;
    using namespace HwColor;

    Uint8 r, g, b;

    // Panel background - draw 4 rectangles AROUND the screen area (don't overwrite LVGL)
    colorToRGB(BACKGROUND, r, g, b);

    // Top strip (above screen)
    boxRGBA(renderer_, 0, 0, PANEL_SIZE, SCREEN_Y, r, g, b, 255);
    // Bottom strip (below screen)
    boxRGBA(renderer_, 0, SCREEN_Y + SCREEN_H, PANEL_SIZE, PANEL_SIZE, r, g, b, 255);
    // Left strip (beside screen)
    boxRGBA(renderer_, 0, SCREEN_Y, SCREEN_X, SCREEN_Y + SCREEN_H, r, g, b, 255);
    // Right strip (beside screen)
    boxRGBA(renderer_, SCREEN_X + SCREEN_W, SCREEN_Y, PANEL_SIZE, SCREEN_Y + SCREEN_H, r, g, b, 255);

    // Panel border with rounded corners
    roundedRectangleRGBA(renderer_, 5, 5, PANEL_SIZE - 5, PANEL_SIZE - 5, 15, 60, 50, 40, 255);

    // Screen area bezel effect
    rectangleRGBA(renderer_, SCREEN_X - 2, SCREEN_Y - 2,
                  SCREEN_X + SCREEN_W + 2, SCREEN_Y + SCREEN_H + 2, 30, 30, 30, 255);
    rectangleRGBA(renderer_, SCREEN_X - 1, SCREEN_Y - 1,
                  SCREEN_X + SCREEN_W + 1, SCREEN_Y + SCREEN_H + 1, 50, 50, 50, 255);
}

void HwSimulator::render() {
    if (!renderer_) return;

    renderPanel();

    for (const auto& btn : buttons_) {
        btn.render(renderer_);
    }

    for (const auto& enc : encoders_) {
        enc.render(renderer_);
    }

    renderLegend();
}

void HwSimulator::handleEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_MOUSEBUTTONDOWN: {
            int mx = event.button.x;
            int my = event.button.y;

            // Check buttons
            for (auto& btn : buttons_) {
                if (btn.hitTest(mx, my)) {
                    btn.pressed = true;
                    // Track active control
                    activeControl_.type = ControlInfo::Type::BUTTON;
                    activeControl_.id = btn.id;
                    activeControl_.pressed = true;
                    activeControl_.name = getControlName(btn.id);
                    activeControlTime_ = SDL_GetTicks();
                    if (buttonCallback_) {
                        buttonCallback_(btn.id, true);
                    }
                    return;
                }
            }

            // Check encoders
            for (auto& enc : encoders_) {
                if (enc.hitTest(mx, my)) {
                    // Check if clicking center button
                    int centerRadius = enc.radius / 3;
                    int dx = mx - enc.x;
                    int dy = my - enc.y;
                    if (enc.btnId >= 0 && (dx * dx + dy * dy) <= (centerRadius * centerRadius)) {
                        enc.pressed = true;
                        // Track active control (button press on encoder)
                        activeControl_.type = ControlInfo::Type::BUTTON;
                        activeControl_.id = enc.btnId;
                        activeControl_.pressed = true;
                        activeControl_.name = getControlName(enc.btnId);
                        activeControlTime_ = SDL_GetTicks();
                        if (buttonCallback_) {
                            buttonCallback_(enc.btnId, true);
                        }
                    } else {
                        // Start dragging for encoder value
                        enc.dragging = true;
                        enc.dragStartY = my;
                        activeEncoder_ = &enc;
                        // Track active control (encoder drag)
                        activeControl_.type = ControlInfo::Type::ENCODER;
                        activeControl_.id = enc.encId;
                        activeControl_.value = enc.value;
                        activeControl_.name = getControlName(enc.encId);
                        activeControlTime_ = SDL_GetTicks();
                    }
                    return;
                }
            }
            break;
        }

        case SDL_MOUSEBUTTONUP: {
            // Release buttons
            for (auto& btn : buttons_) {
                if (btn.pressed) {
                    btn.pressed = false;
                    if (buttonCallback_) {
                        buttonCallback_(btn.id, false);
                    }
                }
            }

            // Release encoders
            for (auto& enc : encoders_) {
                if (enc.pressed) {
                    enc.pressed = false;
                    if (buttonCallback_) {
                        buttonCallback_(enc.btnId, false);
                    }
                }
                enc.dragging = false;
            }
            activeEncoder_ = nullptr;
            break;
        }

        case SDL_MOUSEMOTION: {
            if (activeEncoder_ && activeEncoder_->dragging) {
                int dy = activeEncoder_->dragStartY - event.motion.y;
                activeEncoder_->dragStartY = event.motion.y;

                float delta = dy / 100.0f;

                if (activeEncoder_->isRelative) {
                    // Relative encoder: just send delta
                    if (encoderCallback_) {
                        encoderCallback_(activeEncoder_->encId, delta);
                    }
                } else {
                    // Absolute encoder: update value
                    activeEncoder_->value = std::clamp(activeEncoder_->value + delta, 0.0f, 1.0f);
                    if (encoderCallback_) {
                        encoderCallback_(activeEncoder_->encId, activeEncoder_->value);
                    }
                }
            }
            break;
        }

        case SDL_MOUSEWHEEL: {
            // Find encoder under mouse
            int mx, my;
            SDL_GetMouseState(&mx, &my);

            for (auto& enc : encoders_) {
                if (enc.hitTest(mx, my)) {
                    float delta = event.wheel.y * 0.02f;

                    // Track active control
                    activeControl_.type = ControlInfo::Type::ENCODER;
                    activeControl_.id = enc.encId;
                    activeControl_.name = getControlName(enc.encId);
                    activeControlTime_ = SDL_GetTicks();

                    if (enc.isRelative) {
                        activeControl_.value = delta;
                        if (encoderCallback_) {
                            encoderCallback_(enc.encId, delta);
                        }
                    } else {
                        enc.value = std::clamp(enc.value + delta, 0.0f, 1.0f);
                        activeControl_.value = enc.value;
                        if (encoderCallback_) {
                            encoderCallback_(enc.encId, enc.value);
                        }
                    }
                    break;
                }
            }
            break;
        }
    }
}

void HwSimulator::setEncoderValue(int encId, float value) {
    for (auto& enc : encoders_) {
        if (enc.encId == encId) {
            enc.value = std::clamp(value, 0.0f, 1.0f);
            break;
        }
    }
}

const char* HwSimulator::getControlName(int id) const {
    switch (id) {
        // Buttons
        case HwId::LEFT_TOP:      return "LEFT_TOP";
        case HwId::LEFT_CENTER:   return "LEFT_CENTER";
        case HwId::LEFT_BOTTOM:   return "LEFT_BOTTOM";
        case HwId::BOTTOM_LEFT:   return "BOTTOM_LEFT";
        case HwId::BOTTOM_CENTER: return "BOTTOM_CENTER";
        case HwId::BOTTOM_RIGHT:  return "BOTTOM_RIGHT";
        case HwId::NAV_BTN:       return "NAV_BTN";
        case HwId::MACRO_BTN_1:   return "MACRO_BTN_1";
        case HwId::MACRO_BTN_2:   return "MACRO_BTN_2";
        case HwId::MACRO_BTN_3:   return "MACRO_BTN_3";
        case HwId::MACRO_BTN_4:   return "MACRO_BTN_4";
        case HwId::MACRO_BTN_5:   return "MACRO_BTN_5";
        case HwId::MACRO_BTN_6:   return "MACRO_BTN_6";
        case HwId::MACRO_BTN_7:   return "MACRO_BTN_7";
        case HwId::MACRO_BTN_8:   return "MACRO_BTN_8";
        // Encoders
        case HwId::NAV_ENC:       return "NAV_ENC";
        case HwId::OPT_ENC:       return "OPT_ENC";
        case HwId::MACRO_ENC_1:   return "MACRO_ENC_1";
        case HwId::MACRO_ENC_2:   return "MACRO_ENC_2";
        case HwId::MACRO_ENC_3:   return "MACRO_ENC_3";
        case HwId::MACRO_ENC_4:   return "MACRO_ENC_4";
        case HwId::MACRO_ENC_5:   return "MACRO_ENC_5";
        case HwId::MACRO_ENC_6:   return "MACRO_ENC_6";
        case HwId::MACRO_ENC_7:   return "MACRO_ENC_7";
        case HwId::MACRO_ENC_8:   return "MACRO_ENC_8";
        default:                  return "UNKNOWN";
    }
}

void HwSimulator::renderLegend() {
    using namespace HwLayout;

    // Simple active control indicator in bottom-left corner of panel
    if (activeControl_.type == ControlInfo::Type::NONE) return;

    uint32_t elapsed = SDL_GetTicks() - activeControlTime_;
    if (elapsed > 2000) return;  // Only show for 2 seconds

    // Indicator position (bottom-left, avoiding macro area)
    const int indicatorX = 60;
    const int indicatorY = PANEL_SIZE - 60;
    const int indicatorRadius = 40;

    // Get color from active control
    uint32_t color = 0x808080;
    if (activeControl_.type == ControlInfo::Type::BUTTON) {
        for (const auto& btn : buttons_) {
            if (btn.id == activeControl_.id) {
                color = btn.color;
                break;
            }
        }
        for (const auto& enc : encoders_) {
            if (enc.btnId == activeControl_.id) {
                color = enc.color;
                break;
            }
        }
    } else {
        for (const auto& enc : encoders_) {
            if (enc.encId == activeControl_.id) {
                color = enc.color;
                break;
            }
        }
    }

    Uint8 r, g, b;
    colorToRGB(color, r, g, b);

    // Pulsing effect
    float pulse = 0.7f + 0.3f * sinf(elapsed * 0.01f);
    Uint8 alpha = static_cast<Uint8>(200 * pulse);

    // Glow and indicator
    filledCircleRGBA(renderer_, indicatorX, indicatorY, indicatorRadius + 8, r / 4, g / 4, b / 4, alpha / 3);
    filledCircleRGBA(renderer_, indicatorX, indicatorY, indicatorRadius, r, g, b, alpha);
    aacircleRGBA(renderer_, indicatorX, indicatorY, indicatorRadius, 255, 255, 255, alpha);

    // Type indicator: square for button, ring for encoder
    if (activeControl_.type == ControlInfo::Type::BUTTON) {
        // Small square in center
        int sq = 12;
        boxRGBA(renderer_, indicatorX - sq, indicatorY - sq, indicatorX + sq, indicatorY + sq, 0, 0, 0, 200);
    } else {
        // Ring for encoder
        aacircleRGBA(renderer_, indicatorX, indicatorY, 15, 0, 0, 0, 200);
        aacircleRGBA(renderer_, indicatorX, indicatorY, 10, 0, 0, 0, 200);
    }
}
