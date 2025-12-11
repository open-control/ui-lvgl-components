#pragma once

#include <memory>

#include <lvgl.h>

#include <oc/ui/lvgl/IComponent.hpp>

#include "../widget/ButtonWidget.hpp"
#include "../widget/Label.hpp"

namespace oc::ui::lvgl {

/**
 * @brief Parameter component for binary on/off values
 *
 * Composes a ButtonWidget with a name label below.
 * Takes 100% of parent size and uses grid layout:
 * - Row 0: ButtonWidget (takes remaining space, FR(1), centered)
 * - Row 1: Label (content height)
 *
 * Access inner widgets directly for configuration.
 *
 * Usage:
 * @code
 * auto param = ParameterSwitch(parent);
 * param.button().onColor(0xECA747).setState(true);
 * param.label().setText("Bypass");
 * @endcode
 */
class ParameterSwitch : public IComponent {
public:
    explicit ParameterSwitch(lv_obj_t* parent);
    ~ParameterSwitch();

    // Move only
    ParameterSwitch(ParameterSwitch&& other) noexcept;
    ParameterSwitch& operator=(ParameterSwitch&& other) noexcept;
    ParameterSwitch(const ParameterSwitch&) = delete;
    ParameterSwitch& operator=(const ParameterSwitch&) = delete;

    // IComponent
    lv_obj_t* getElement() const override { return container_; }
    void show() override;
    void hide() override;
    bool isVisible() const override;

    // Widget access
    ButtonWidget& button() { return *button_; }
    const ButtonWidget& button() const { return *button_; }
    Label& label() { return *label_; }
    const Label& label() const { return *label_; }

private:
    void createUI(lv_obj_t* parent);
    void cleanup();

    lv_obj_t* container_ = nullptr;
    std::unique_ptr<ButtonWidget> button_;
    std::unique_ptr<Label> label_;
};

}  // namespace oc::ui::lvgl
