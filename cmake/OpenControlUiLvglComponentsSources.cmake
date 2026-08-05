# Canonical production source inventory for UI-LVGL Components consumers.
set(OC_UI_LVGL_COMPONENTS_SOURCE_PATHS
    src/component/ParameterEnum.cpp
    src/component/ParameterKnob.cpp
    src/component/ParameterSwitch.cpp
    src/widget/ButtonWidget.cpp
    src/widget/EnumWidget.cpp
    src/widget/KnobWidget.cpp
    src/widget/Label.cpp
    src/widget/StateIndicator.cpp
    src/widget/VirtualList.cpp
)

set(OC_UI_LVGL_COMPONENTS_SOURCES ${OC_UI_LVGL_COMPONENTS_SOURCE_PATHS})
list(TRANSFORM OC_UI_LVGL_COMPONENTS_SOURCES
    PREPEND "${CMAKE_CURRENT_LIST_DIR}/../")
