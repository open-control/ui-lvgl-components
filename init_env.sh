#!/bin/bash
# Initialize environment: PlatformIO deps + Zig + CMake (100% self-contained)
set -e

# =============================================================================
# Colors & formatting
# =============================================================================
RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'
CYAN='\033[0;36m'; GRAY='\033[0;90m'; BOLD='\033[1m'; NC='\033[0m'
CHECKMARK="${GREEN}✓${NC}"; CROSS="${RED}✗${NC}"
HIDE_CURSOR='\033[?25l'; SHOW_CURSOR='\033[?25h'

# =============================================================================
# Spinner (cursor hidden)
# =============================================================================
SPINNER_PID=""
spinner() {
    local chars="⠋⠙⠹⠸⠼⠴⠦⠧⠇⠏"
    while :; do
        for (( i=0; i<${#chars}; i++ )); do
            printf "\r  ${CYAN}%s${NC} %s" "${chars:$i:1}" "$1"
            sleep 0.1
        done
    done
}

start_spinner() { printf "${HIDE_CURSOR}"; spinner "$1" & SPINNER_PID=$!; }
stop_spinner() {
    [[ -n "$SPINNER_PID" ]] && kill "$SPINNER_PID" 2>/dev/null
    SPINNER_PID=""
    printf "\r\033[K${SHOW_CURSOR}"
}
trap 'printf "${SHOW_CURSOR}"; stop_spinner' EXIT

# =============================================================================
# Find project root
# =============================================================================
find_project_root() {
    local dir="$PWD"
    while [[ "$dir" != "/" ]]; do
        [[ -f "$dir/platformio.ini" ]] && { echo "$dir"; return 0; }
        dir="$(dirname "$dir")"
    done
    echo "Error: platformio.ini not found" >&2
    return 1
}

PROJECT_ROOT=$(find_project_root)
cd "$PROJECT_ROOT"
TOOLS_DIR="$PROJECT_ROOT/tools"
mkdir -p "$TOOLS_DIR"

echo ""
echo -e "${BOLD}Initializing Environment${NC}"
echo -e "${GRAY}─────────────────────────────────────────${NC}"

# =============================================================================
# Detect platform
# =============================================================================
case "$OSTYPE" in
    msys*|mingw*|cygwin*)
        PLATFORM="windows"
        ZIG_PLATFORM="x86_64-windows"; ZIG_EXT="zip"
        CMAKE_PLATFORM="windows-x86_64"; CMAKE_EXT="zip"
        EXE_EXT=".exe"
        ;;
    linux*)
        PLATFORM="linux"
        ZIG_PLATFORM="x86_64-linux"; ZIG_EXT="tar.xz"
        CMAKE_PLATFORM="linux-x86_64"; CMAKE_EXT="tar.gz"
        EXE_EXT=""
        ;;
    darwin*)
        PLATFORM="macos"
        ZIG_PLATFORM="x86_64-macos"; ZIG_EXT="tar.xz"
        CMAKE_PLATFORM="macos-universal"; CMAKE_EXT="tar.gz"
        EXE_EXT=""
        ;;
    *)
        echo -e "  ${CROSS} Unsupported platform: $OSTYPE"
        exit 1
        ;;
esac

# =============================================================================
# Download and extract tool
# =============================================================================
download_tool() {
    local name="$1" url="$2" archive="$3" extract_dir="$4"

    start_spinner "Downloading ${name}..."
    curl -fsSL "$url" -o "$archive"
    stop_spinner

    start_spinner "Extracting ${name}..."
    case "$archive" in
        *.zip)    unzip -q "$archive" -d "$TOOLS_DIR" ;;
        *.tar.gz) tar -xzf "$archive" -C "$TOOLS_DIR" ;;
        *.tar.xz) tar -xf "$archive" -C "$TOOLS_DIR" ;;
    esac
    rm -f "$archive"
    stop_spinner
}

# =============================================================================
# Setup Ninja (fast build system)
# =============================================================================
NINJA_VERSION="1.13.2"
NINJA_DIR="$TOOLS_DIR/ninja-${NINJA_VERSION}"
NINJA_BIN="$NINJA_DIR/ninja${EXE_EXT}"

case "$PLATFORM" in
    windows) NINJA_PLATFORM="win" ;;
    linux)   NINJA_PLATFORM="linux" ;;
    macos)   NINJA_PLATFORM="mac" ;;
esac

if [[ -x "$NINJA_BIN" ]]; then
    echo -e "  ${CHECKMARK} Ninja ${NINJA_VERSION} ${GRAY}(cached)${NC}"
else
    mkdir -p "$NINJA_DIR"
    NINJA_URL="https://github.com/ninja-build/ninja/releases/download/v${NINJA_VERSION}/ninja-${NINJA_PLATFORM}.zip"
    start_spinner "Downloading Ninja ${NINJA_VERSION}..."
    curl -fsSL "$NINJA_URL" -o "$TOOLS_DIR/ninja.zip"
    stop_spinner
    start_spinner "Extracting Ninja..."
    unzip -q "$TOOLS_DIR/ninja.zip" -d "$NINJA_DIR"
    rm -f "$TOOLS_DIR/ninja.zip"
    stop_spinner
    echo -e "  ${CHECKMARK} Ninja ${NINJA_VERSION} ${GRAY}(downloaded)${NC}"
fi

# =============================================================================
# Setup CMake
# =============================================================================
CMAKE_VERSION="4.2.1"
CMAKE_DIR="$TOOLS_DIR/cmake-${CMAKE_VERSION}-${CMAKE_PLATFORM}"
CMAKE_BIN="$CMAKE_DIR/bin/cmake${EXE_EXT}"

if [[ -x "$CMAKE_BIN" ]]; then
    echo -e "  ${CHECKMARK} CMake ${CMAKE_VERSION} ${GRAY}(cached)${NC}"
else
    CMAKE_URL="https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-${CMAKE_PLATFORM}.${CMAKE_EXT}"
    download_tool "CMake ${CMAKE_VERSION}" "$CMAKE_URL" "$TOOLS_DIR/cmake.${CMAKE_EXT}" "$CMAKE_DIR"
    echo -e "  ${CHECKMARK} CMake ${CMAKE_VERSION} ${GRAY}(downloaded)${NC}"
fi

# =============================================================================
# Setup Zig
# =============================================================================
ZIG_VERSION="0.15.2"
ZIG_DIR="$TOOLS_DIR/zig-${ZIG_PLATFORM}-${ZIG_VERSION}"
ZIG_BIN="$ZIG_DIR/zig${EXE_EXT}"

if [[ -x "$ZIG_BIN" ]]; then
    echo -e "  ${CHECKMARK} Zig ${ZIG_VERSION} ${GRAY}(cached)${NC}"
else
    ZIG_URL="https://ziglang.org/download/${ZIG_VERSION}/zig-${ZIG_PLATFORM}-${ZIG_VERSION}.${ZIG_EXT}"
    download_tool "Zig ${ZIG_VERSION}" "$ZIG_URL" "$TOOLS_DIR/zig.${ZIG_EXT}" "$ZIG_DIR"
    echo -e "  ${CHECKMARK} Zig ${ZIG_VERSION} ${GRAY}(downloaded)${NC}"
fi

# Create Zig compiler wrapper scripts
if [[ "$PLATFORM" == "windows" ]]; then
    # Convert to Windows path format
    ZIG_DIR_WIN=$(cygpath -w "$ZIG_DIR")
    cat > "$TOOLS_DIR/zig-cc.cmd" << ZIGCC
@echo off
"${ZIG_DIR_WIN}\\zig.exe" cc %*
ZIGCC
    cat > "$TOOLS_DIR/zig-c++.cmd" << ZIGCXX
@echo off
"${ZIG_DIR_WIN}\\zig.exe" c++ %*
ZIGCXX
    cat > "$TOOLS_DIR/zig-ar.cmd" << ZIGAR
@echo off
"${ZIG_DIR_WIN}\\zig.exe" ar %*
ZIGAR
else
    cat > "$TOOLS_DIR/zig-cc" << ZIGCC
#!/bin/sh
"$ZIG_DIR/zig" cc "\$@"
ZIGCC
    chmod +x "$TOOLS_DIR/zig-cc"
    cat > "$TOOLS_DIR/zig-c++" << ZIGCXX
#!/bin/sh
"$ZIG_DIR/zig" c++ "\$@"
ZIGCXX
    chmod +x "$TOOLS_DIR/zig-c++"
    cat > "$TOOLS_DIR/zig-ar" << ZIGAR
#!/bin/sh
"$ZIG_DIR/zig" ar "\$@"
ZIGAR
    chmod +x "$TOOLS_DIR/zig-ar"
fi

# =============================================================================
# Check PlatformIO
# =============================================================================
if ! command -v pio &> /dev/null; then
    echo -e "  ${CROSS} PlatformIO CLI not found"
    echo -e "     Install: ${CYAN}pip install platformio${NC}"
    exit 1
fi
echo -e "  ${CHECKMARK} PlatformIO CLI"

# =============================================================================
# Install PlatformIO dependencies
# =============================================================================
start_spinner "Installing dependencies..."
pio pkg install > /dev/null 2>&1
stop_spinner
echo -e "  ${CHECKMARK} PlatformIO dependencies"

# =============================================================================
# Find and verify PlatformIO environment
# =============================================================================
PIO_LIBDEPS_DIR="$PROJECT_ROOT/.pio/libdeps"
PIO_ENV_DIR=$(ls -d "$PIO_LIBDEPS_DIR"/*/ 2>/dev/null | head -1)

if [[ -z "$PIO_ENV_DIR" ]]; then
    echo -e "  ${CROSS} No environment found in .pio/libdeps/"
    exit 1
fi

check_dep() {
    local dep="$1"
    [[ -d "$PIO_ENV_DIR/$dep" ]] && return 0
    [[ -f "$PIO_ENV_DIR/${dep}.pio-link" ]] && return 0
    return 1
}

DEPS=("framework" "ui-lvgl" "lvgl")
MISSING=()
for dep in "${DEPS[@]}"; do
    check_dep "$dep" || MISSING+=("$dep")
done

if [[ ${#MISSING[@]} -gt 0 ]]; then
    echo -e "  ${CROSS} Missing: ${MISSING[*]}"
    exit 1
fi
echo -e "  ${CHECKMARK} Dependencies verified"

# =============================================================================
# Write environment file
# =============================================================================
cat > "$PROJECT_ROOT/.pio_env" << EOF
PIO_ENV_DIR=$PIO_ENV_DIR
ZIG_DIR=$ZIG_DIR
CMAKE_DIR=$CMAKE_DIR
NINJA_DIR=$NINJA_DIR
EOF

echo ""
echo -e "${GREEN}Environment ready${NC} ${GRAY}(100% self-contained)${NC}"
echo ""
