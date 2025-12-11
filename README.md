# ui-lvgl-components

Reusable LVGL UI components for the **open-control** framework - an open-source ecosystem for building hardware audio controllers with embedded displays.

## Open-Control Ecosystem

| Repository | Description |
|------------|-------------|
| [framework](https://github.com/open-control/framework) | Core framework (parameter management, MIDI, etc.) |
| [ui-lvgl](https://github.com/open-control/ui-lvgl) | LVGL integration layer |
| **ui-lvgl-components** | Reusable UI widgets (this repo) |
| [hal-common](https://github.com/open-control/hal-common) | Hardware abstraction - common |
| [hal-teensy](https://github.com/open-control/hal-teensy) | Hardware abstraction - Teensy |

## Features

- **Thin wrappers** - Full LVGL compatibility, zero friction
- **Fluent API** - Chainable configuration methods
- **Move semantics** - Proper C++17 resource management
- **Theme defaults** - Sensible defaults with easy overrides

## Widgets

| Widget | Description |
|--------|-------------|
| `KnobWidget` | Rotary arc knob with value indicator (auto-flash on value change) |
| `ListItemWidget` | Container with indicator line and content area (manual flash) |
| `ButtonWidget` | Toggle button with ON/OFF states |
| `StateIndicator` | Circular LED-style state indicator |
| `Label` | Label with optional auto-scroll for overflow text |

## Quick Start - SDL Demo

The SDL demo provides a desktop environment to develop and test UI components without hardware.

### Prerequisites

- **Git** (with Git Bash on Windows)
- **VSCode** + **PlatformIO extension**

#### Linux only: Graphics headers

On Linux, SDL2 needs graphics development headers to enable display support. The build tools (Zig, CMake, Ninja) are downloaded automatically, but the graphics headers must be installed from your system package manager.

**Fedora/RHEL:**
```bash
# Wayland (recommended)
sudo dnf install wayland-devel libxkbcommon-devel mesa-libEGL-devel

# Or X11
sudo dnf install libX11-devel libXext-devel libXrandr-devel libXcursor-devel libXi-devel
```

**Ubuntu/Debian:**
```bash
# Wayland (recommended)
sudo apt install libwayland-dev libxkbcommon-dev libegl1-mesa-dev

# Or X11
sudo apt install libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxi-dev
```

**Why is this needed?**

| Platform | Compile-time headers | Runtime libraries |
|----------|---------------------|-------------------|
| **Windows** | Bundled in Zig (MinGW) | Always present (user32.dll, gdi32.dll) |
| **Linux** | System packages (`*-devel`) | Always present on desktop distros |
| **macOS** | Xcode frameworks | Always present |

Windows appears "self-contained" because Zig bundles MinGW headers and Windows always has the Win32 graphics DLLs. On Linux, the runtime libraries (libwayland, libEGL) are present on any desktop system, but the development headers are not installed by default.

### Setup

```bash
# Create workspace and clone repos
mkdir open-control && cd open-control
git clone https://github.com/open-control/framework.git
git clone https://github.com/open-control/ui-lvgl.git
git clone https://github.com/open-control/ui-lvgl-components.git
```

### Build & Run

> **Note:** Run scripts from **VSCode's PlatformIO Terminal** (not external Git Bash).
> Open with: `Ctrl+Shift+P` → "PlatformIO: Open PlatformIO Core CLI"

```bash
cd ui-lvgl-components

# Build the SDL demo (first run downloads tools ~200MB)
./build_demo.sh

# Run the demo
./run_demo.sh
```

> **First build is slow (2-4 min)** - CMake runs hundreds of feature detection tests for SDL2.
> Subsequent builds are fast (~5s for incremental changes).

### Scripts

| Script | Description |
|--------|-------------|
| `./build_demo.sh` | Build SDL demo (auto-downloads tools if needed) |
| `./run_demo.sh` | Run the demo (builds if needed) |
| `./clean_env.sh` | Clean everything (tools + build) |
| `./clean_env.sh --build-only` | Clean build artifacts only |
| `./init_env.sh` | Initialize environment manually |

## Build System

The build system downloads its toolchain automatically:

| Tool | Version | Purpose |
|------|---------|---------|
| **Zig** | 0.15.2 | C/C++ compiler (drop-in replacement) |
| **CMake** | 4.2.1 | Build configuration |
| **Ninja** | 1.13.2 | Fast build system |
| **SDL2** | 2.30.10 | Fetched by CMake (FetchContent) |

All tools are downloaded to `tools/` and cached between builds (~200MB on first run).

### Why Zig?

Zig provides a hermetic C/C++ toolchain that works identically across Windows, Linux, and macOS - no Visual Studio, no MinGW, no Xcode required.

### Platform differences

> **Note:** macOS support is implemented but remains untested.

```
┌─────────────────────────────────────────────────────────────────┐
│                        SDL Demo Binary                          │
├─────────────────────────────────────────────────────────────────┤
│  Compile time                │  Runtime                         │
├──────────────────────────────┼──────────────────────────────────┤
│  Zig (bundled)               │  System graphics libs            │
│  CMake (bundled)             │  - Windows: user32.dll, gdi32.dll│
│  Ninja (bundled)             │  - Linux: libwayland, libEGL     │
│  SDL2 (fetched)              │  - macOS: Cocoa frameworks       │
│  + Linux: *-devel headers    │                                  │
└──────────────────────────────┴──────────────────────────────────┘
```

The binary always dynamically links to system graphics libraries (required to communicate with the display server). On Windows, headers are bundled in Zig. On Linux, you need to install development headers (see Prerequisites).

## Installation (PlatformIO)

For embedded targets (Teensy, ESP32, etc.):

```ini
lib_deps =
    open-control/ui-lvgl-components@^0.1.0
```

## Usage

### KnobWidget

```cpp
#include <oc/ui/lvgl/widget/KnobWidget.hpp>

using namespace oc::ui::lvgl;

KnobWidget knob(parent);
knob.trackColor(0xFCEB23)
    .valueColor(0x909090)
    .flashColor(0xECA747)
    .centered(true);
knob.setValue(0.5f);  // Auto-flash on value change
```

### ListItemWidget

```cpp
#include <oc/ui/lvgl/widget/ListItemWidget.hpp>

ListItemWidget item(parent);
item.lineColor(0x606060)
    .flashColor(0xECA747);

// Add content to inner()
lv_obj_t* label = lv_label_create(item.inner());
lv_label_set_text(label, "Sine Wave");

item.triggerFlash();  // Manual flash
```

### ButtonWidget

```cpp
#include <oc/ui/lvgl/widget/ButtonWidget.hpp>

ButtonWidget button(parent);
button.offColor(0x333333)
      .onColor(0xECA747)
      .textOffColor(0xFFFFFF)
      .textOnColor(0x292929);
button.setText("OFF");
button.setState(true);  // Changes to ON state
```

### StateIndicator

```cpp
#include <oc/ui/lvgl/widget/StateIndicator.hpp>

StateIndicator indicator(parent, 12);
indicator.color(StateIndicator::State::OFF, 0x606060)
         .color(StateIndicator::State::ACTIVE, 0x00FF00)
         .opacity(StateIndicator::State::OFF, LV_OPA_60);
indicator.setState(StateIndicator::State::ACTIVE);
```

### Direct LVGL Access

```cpp
// Implicit conversion to lv_obj_t*
lv_obj_set_style_bg_color(knob, lv_color_hex(0xFF0000), 0);

// Add events
lv_obj_add_event_cb(knob, my_callback, LV_EVENT_CLICKED, nullptr);

// Access sub-elements
lv_obj_set_style_arc_width(knob.getArc(), 12, LV_PART_MAIN);
```

## Project Structure

```
ui-lvgl-components/
├── src/                    # Library source
│   └── widget/             # Widget implementations
├── include/                # Public headers
│   └── oc/ui/lvgl/
│       ├── widget/         # Widget classes
│       └── theme/          # Theme definitions
├── examples/
│   └── sdl_demo/           # Desktop SDL demo
│       ├── src/            # Demo source
│       └── lv_conf.h       # LVGL configuration
├── tools/                  # Downloaded toolchain (gitignored)
│   └── zig-toolchain.cmake # CMake toolchain file
├── build_demo.sh           # Build script
├── run_demo.sh             # Run script
├── clean_env.sh            # Clean script
└── init_env.sh             # Environment init
```

## License

MIT
