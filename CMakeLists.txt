cmake_minimum_required(VERSION 3.29)

project(open_control_ui_lvgl_components VERSION 0.2.0 LANGUAGES CXX)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/OpenControlUiLvglComponentsSources.cmake")

add_library(oc_ui_lvgl_components INTERFACE)
target_sources(oc_ui_lvgl_components
    INTERFACE ${OC_UI_LVGL_COMPONENTS_SOURCES})
target_include_directories(oc_ui_lvgl_components
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(oc_ui_lvgl_components INTERFACE cxx_std_17)
