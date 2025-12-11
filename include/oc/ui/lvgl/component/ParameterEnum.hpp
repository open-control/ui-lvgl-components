#pragma once

#include <memory>

#include <lvgl.h>

#include <oc/ui/lvgl/IComponent.hpp>

#include "../widget/EnumWidget.hpp"
#include "../widget/Label.hpp"

namespace oc::ui::lvgl {

/**
 * @brief Parameter component for discrete/enumerated values
 *
 * Composes an EnumWidget (value display) with a name label below.
 * Takes 100% of parent size and uses grid layout:
 * - Row 0: EnumWidget (takes remaining space, FR(1))
 * - Row 1: Label (content height)
 *
 * Access inner widgets directly for configuration.
 *
 * Usage:
 * @code
 * auto param = ParameterEnum(parent);
 * param.enumWidget().flashColor(0xECA747);
 * param.valueLabel().setText("Sawtooth");
 * param.nameLabel().setText("Waveform");
 * @endcode
 */
class ParameterEnum : public IComponent {
public:
    explicit ParameterEnum(lv_obj_t* parent);
    ~ParameterEnum();

    // Move only
    ParameterEnum(ParameterEnum&& other) noexcept;
    ParameterEnum& operator=(ParameterEnum&& other) noexcept;
    ParameterEnum(const ParameterEnum&) = delete;
    ParameterEnum& operator=(const ParameterEnum&) = delete;

    // IComponent
    lv_obj_t* getElement() const override { return container_; }
    void show() override;
    void hide() override;
    bool isVisible() const override;

    // Widget access
    EnumWidget& enumWidget() { return *enum_widget_; }
    const EnumWidget& enumWidget() const { return *enum_widget_; }
    Label& valueLabel() { return *value_label_; }
    const Label& valueLabel() const { return *value_label_; }
    Label& nameLabel() { return *name_label_; }
    const Label& nameLabel() const { return *name_label_; }

private:
    void createUI(lv_obj_t* parent);
    void cleanup();

    lv_obj_t* container_ = nullptr;
    std::unique_ptr<EnumWidget> enum_widget_;
    std::unique_ptr<Label> value_label_;
    std::unique_ptr<Label> name_label_;
};

}  // namespace oc::ui::lvgl
