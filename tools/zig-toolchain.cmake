# CMake toolchain for Zig compiler
# Requires ZIG_DIR environment variable to be set

# Detect host platform and set appropriate system name
if(CMAKE_HOST_WIN32)
    set(CMAKE_SYSTEM_NAME Windows)
elseif(CMAKE_HOST_APPLE)
    set(CMAKE_SYSTEM_NAME Darwin)
elseif(CMAKE_HOST_UNIX)
    set(CMAKE_SYSTEM_NAME Linux)
else()
    message(FATAL_ERROR "Unsupported host platform")
endif()

set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Find zig executable
if(DEFINED ENV{ZIG_DIR})
    set(ZIG_DIR "$ENV{ZIG_DIR}")
else()
    message(FATAL_ERROR "ZIG_DIR environment variable not set")
endif()

# Platform-specific executable extension
if(CMAKE_HOST_WIN32)
    set(ZIG_EXE "${ZIG_DIR}/zig.exe")
else()
    set(ZIG_EXE "${ZIG_DIR}/zig")
endif()

if(NOT EXISTS "${ZIG_EXE}")
    message(FATAL_ERROR "Zig not found at: ${ZIG_EXE}")
endif()

# Wrapper scripts location
get_filename_component(TOOLCHAIN_DIR "${CMAKE_CURRENT_LIST_FILE}" DIRECTORY)

# Use zig as C and C++ compiler
if(CMAKE_HOST_WIN32)
    set(CMAKE_C_COMPILER "${TOOLCHAIN_DIR}/zig-cc.cmd")
    set(CMAKE_CXX_COMPILER "${TOOLCHAIN_DIR}/zig-c++.cmd")
else()
    set(CMAKE_C_COMPILER "${TOOLCHAIN_DIR}/zig-cc")
    set(CMAKE_CXX_COMPILER "${TOOLCHAIN_DIR}/zig-c++")
endif()

# Skip compiler checks (zig handles everything)
set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_CXX_COMPILER_WORKS TRUE)
set(CMAKE_C_ABI_COMPILED TRUE)
set(CMAKE_CXX_ABI_COMPILED TRUE)

# Use native target
set(CMAKE_C_COMPILER_TARGET "native")
set(CMAKE_CXX_COMPILER_TARGET "native")

# AR and RANLIB using zig wrappers
if(CMAKE_HOST_WIN32)
    set(CMAKE_AR "${TOOLCHAIN_DIR}/zig-ar.cmd" CACHE FILEPATH "")
    set(CMAKE_C_COMPILER_AR "${TOOLCHAIN_DIR}/zig-ar.cmd" CACHE FILEPATH "")
    set(CMAKE_CXX_COMPILER_AR "${TOOLCHAIN_DIR}/zig-ar.cmd" CACHE FILEPATH "")
else()
    set(CMAKE_AR "${TOOLCHAIN_DIR}/zig-ar" CACHE FILEPATH "")
    set(CMAKE_C_COMPILER_AR "${TOOLCHAIN_DIR}/zig-ar" CACHE FILEPATH "")
    set(CMAKE_CXX_COMPILER_AR "${TOOLCHAIN_DIR}/zig-ar" CACHE FILEPATH "")
endif()
set(CMAKE_RANLIB "true" CACHE FILEPATH "")
