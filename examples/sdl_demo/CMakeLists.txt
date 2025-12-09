cmake_minimum_required(VERSION 3.14)
project(ui_lvgl_demo C CXX)

# ==============================================================================
# C/C++ Standards
# ==============================================================================
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==============================================================================
# Find project root and PlatformIO dependencies
# ==============================================================================
# Project root contains platformio.ini
set(PROJECT_ROOT "${CMAKE_SOURCE_DIR}/../..")

# Read PlatformIO environment from .pio_env (created by init_env.sh)
if(EXISTS "${PROJECT_ROOT}/.pio_env")
    file(STRINGS "${PROJECT_ROOT}/.pio_env" PIO_ENV_CONTENT)
    foreach(line ${PIO_ENV_CONTENT})
        if(line MATCHES "^PIO_ENV_DIR=(.*)$")
            set(PIO_ENV_DIR "${CMAKE_MATCH_1}")
        endif()
    endforeach()
endif()

# Fallback: find first available environment
if(NOT PIO_ENV_DIR OR NOT EXISTS "${PIO_ENV_DIR}")
    file(GLOB PIO_ENV_DIRS "${PROJECT_ROOT}/.pio/libdeps/*/")
    list(GET PIO_ENV_DIRS 0 PIO_ENV_DIR)
endif()

if(NOT EXISTS "${PIO_ENV_DIR}")
    message(FATAL_ERROR "PlatformIO dependencies not found. Run ./init_env.sh first.")
endif()

message(STATUS "Using PlatformIO deps from: ${PIO_ENV_DIR}")

# ==============================================================================
# Resolve PlatformIO dependency (handles both dirs and .pio-link files)
# ==============================================================================
function(resolve_pio_dep DEP_NAME OUT_VAR)
    set(DEP_DIR "${PIO_ENV_DIR}/${DEP_NAME}")
    set(DEP_LINK "${PIO_ENV_DIR}/${DEP_NAME}.pio-link")

    # Direct directory exists
    if(EXISTS "${DEP_DIR}" AND IS_DIRECTORY "${DEP_DIR}")
        set(${OUT_VAR} "${DEP_DIR}" PARENT_SCOPE)
        return()
    endif()

    # .pio-link file exists (PlatformIO symlink on Windows)
    if(EXISTS "${DEP_LINK}")
        file(READ "${DEP_LINK}" LINK_CONTENT)
        # Extract URI from JSON: "uri": "symlink://../path"
        string(REGEX MATCH "\"uri\": *\"symlink://([^\"]+)\"" _ "${LINK_CONTENT}")
        if(CMAKE_MATCH_1)
            # Resolve relative path from project root
            get_filename_component(RESOLVED_PATH "${PROJECT_ROOT}/${CMAKE_MATCH_1}" ABSOLUTE)
            set(${OUT_VAR} "${RESOLVED_PATH}" PARENT_SCOPE)
            return()
        endif()
    endif()

    message(FATAL_ERROR "${DEP_NAME} not found in PlatformIO deps")
endfunction()

# ==============================================================================
# Dependencies from PlatformIO
# ==============================================================================
resolve_pio_dep("lvgl" LVGL_DIR)
resolve_pio_dep("ui-lvgl" UI_LVGL_DIR)
set(UI_LVGL_COMPONENTS_DIR "${PROJECT_ROOT}")

message(STATUS "LVGL: ${LVGL_DIR}")
message(STATUS "ui-lvgl: ${UI_LVGL_DIR}")

# ==============================================================================
# SDL2 (desktop only - via FetchContent)
# ==============================================================================
include(FetchContent)
set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/deps")

message(STATUS "Fetching SDL2...")
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-2.30.10
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)

# SDL2 build - static lib, no tests, GPU acceleration enabled
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL2_DISABLE_SDL2MAIN ON CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)

# Skip pkg-config (self-contained)
set(PKG_CONFIG_EXECUTABLE "" CACHE STRING "" FORCE)

FetchContent_MakeAvailable(SDL2)
FetchContent_GetProperties(SDL2 SOURCE_DIR SDL2_SOURCE_DIR)
set(SDL2_INCLUDE_DIR "${SDL2_SOURCE_DIR}/include")

# ==============================================================================
# LVGL (from PlatformIO)
# ==============================================================================
set(CONFIG_LV_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_USE_THORVG_INTERNAL OFF CACHE BOOL "" FORCE)

add_subdirectory(${LVGL_DIR} ${CMAKE_BINARY_DIR}/lvgl)

target_include_directories(lvgl PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${SDL2_INCLUDE_DIR}
)
target_compile_definitions(lvgl PUBLIC LV_CONF_INCLUDE_SIMPLE)

# ==============================================================================
# ui-lvgl-components sources
# ==============================================================================
file(GLOB_RECURSE UI_LVGL_COMPONENTS_SOURCES
    "${UI_LVGL_COMPONENTS_DIR}/src/*.cpp"
)

# ==============================================================================
# Main executable
# ==============================================================================
set(MAIN_SOURCES
    src/main.cpp
    src/hal/hal.c
    src/mouse_cursor_icon.c
    ${UI_LVGL_COMPONENTS_SOURCES}
)

add_executable(demo ${MAIN_SOURCES})

target_include_directories(demo PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${UI_LVGL_COMPONENTS_DIR}/include
    ${UI_LVGL_DIR}/src
    ${SDL2_INCLUDE_DIR}
)

target_link_libraries(demo PRIVATE
    lvgl
    SDL2::SDL2-static
)

# ==============================================================================
# Platform-specific settings
# ==============================================================================
if(WIN32)
    if(MSVC)
        target_link_options(demo PRIVATE "/SUBSYSTEM:CONSOLE")
    else()
        target_link_options(demo PRIVATE "-mconsole")
    endif()
    target_link_libraries(demo PRIVATE winmm imm32 setupapi version dwmapi)
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(demo PRIVATE m pthread)
endif()

# ==============================================================================
# Output directory
# ==============================================================================
set_target_properties(demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
)

# ==============================================================================
# Debug options
# ==============================================================================
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug mode enabled")
    if(NOT MSVC)
        target_compile_options(demo PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()
