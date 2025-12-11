#pragma once

#include <memory>

#include <lvgl.h>

#include <oc/ui/lvgl/IComponent.hpp>

#include "../widget/KnobWidget.hpp"
#include "../widget/Label.hpp"

namespace oc::ui::lvgl {

/**
 * @brief Parameter knob component for continuous values
 *
 * Composes a KnobWidget with a name label below.
 * Takes 100% of parent size and uses grid layout:
 * - Row 0: KnobWidget (takes remaining space, FR(1))
 * - Row 1: Label (content height)
 *
 * Access inner widgets directly for configuration.
 *
 * Usage:
 * @code
 * auto param = ParameterKnob(parent);
 * param.knob().trackColor(0xFCEB23).centered(true).setValue(0.5f);
 * param.label().setText("Pan");
 * @endcode
 */
class ParameterKnob : public IComponent {
public:
    explicit ParameterKnob(lv_obj_t* parent);
    ~ParameterKnob();

    // Move only
    ParameterKnob(ParameterKnob&& other) noexcept;
    ParameterKnob& operator=(ParameterKnob&& other) noexcept;
    ParameterKnob(const ParameterKnob&) = delete;
    ParameterKnob& operator=(const ParameterKnob&) = delete;

    // IComponent
    lv_obj_t* getElement() const override { return container_; }
    void show() override;
    void hide() override;
    bool isVisible() const override;

    // Widget access
    KnobWidget& knob() { return *knob_; }
    const KnobWidget& knob() const { return *knob_; }
    Label& label() { return *label_; }
    const Label& label() const { return *label_; }

private:
    void createUI(lv_obj_t* parent);
    void cleanup();

    lv_obj_t* container_ = nullptr;
    std::unique_ptr<KnobWidget> knob_;
    std::unique_ptr<Label> label_;
};

}  // namespace oc::ui::lvgl
