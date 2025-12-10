#!/bin/bash
# Build SDL demo (100% self-contained: Zig + CMake)
set -e

# =============================================================================
# Colors & formatting
# =============================================================================
RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'
CYAN='\033[0;36m'; GRAY='\033[0;90m'; BOLD='\033[1m'; NC='\033[0m'
CHECKMARK="${GREEN}✓${NC}"; CROSS="${RED}✗${NC}"; ARROW="${CYAN}→${NC}"
HIDE_CURSOR='\033[?25l'; SHOW_CURSOR='\033[?25h'

# =============================================================================
# Spinner with timer (cursor hidden)
# =============================================================================
SPINNER_PID=""
START_TIME=""

spinner() {
    local chars="⠋⠙⠹⠸⠼⠴⠦⠧⠇⠏"
    while :; do
        local elapsed=$((SECONDS - START_TIME))
        for (( i=0; i<${#chars}; i++ )); do
            printf "\r  ${CYAN}%s${NC} %s ${GRAY}(%ds)${NC}" "${chars:$i:1}" "$1" "$elapsed"
            sleep 0.1
        done
    done
}

start_spinner() { printf "${HIDE_CURSOR}"; START_TIME=$SECONDS; spinner "$1" & SPINNER_PID=$!; }
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

# =============================================================================
# Check environment
# =============================================================================
if [[ ! -f ".pio_env" ]]; then
    echo -e "${YELLOW}Environment not initialized. Running init_env.sh...${NC}"
    ./init_env.sh
fi

# Load environment and export for CMake subprocesses
source ".pio_env"
export ZIG_DIR

# Use local tools
CMAKE="$CMAKE_DIR/bin/cmake"
NINJA="$NINJA_DIR/ninja"
[[ "$OSTYPE" == msys* || "$OSTYPE" == mingw* || "$OSTYPE" == cygwin* ]] && { CMAKE="$CMAKE.exe"; NINJA="$NINJA.exe"; }
export PATH="$NINJA_DIR:$PATH"

echo ""
echo -e "${BOLD}Building SDL Demo${NC}"
echo -e "${GRAY}─────────────────────────────────────────${NC}"

# =============================================================================
# Configuration
# =============================================================================
BUILD_TYPE="${1:-Release}"
DEMO_DIR="$PROJECT_ROOT/examples/sdl_demo"
BUILD_DIR="$DEMO_DIR/build"
TOOLCHAIN_FILE="$PROJECT_ROOT/tools/zig-toolchain.cmake"

echo -e "  ${ARROW} Type: ${CYAN}${BUILD_TYPE}${NC}"
echo -e "  ${ARROW} Compiler: ${CYAN}Zig${NC}"

# =============================================================================
# CMake Configure
# =============================================================================
mkdir -p "$BUILD_DIR"

NEED_CONFIGURE=false
[[ ! -f "$BUILD_DIR/CMakeCache.txt" ]] && NEED_CONFIGURE=true

if $NEED_CONFIGURE; then
    start_spinner "Configuring CMake..."

    "$CMAKE" -S "$DEMO_DIR" -B "$BUILD_DIR" \
        -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -G "Ninja" \
        -Wno-dev \
        > "$BUILD_DIR/cmake_configure.log" 2>&1 || {
        stop_spinner
        echo -e "  ${CROSS} CMake configure failed"
        echo -e "     ${GRAY}See: examples/sdl_demo/build/cmake_configure.log${NC}"
        exit 1
    }

    stop_spinner
    echo -e "  ${CHECKMARK} CMake configured"
fi

# =============================================================================
# Build (parallel)
# =============================================================================
NPROC=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
start_spinner "Compiling (${NPROC} threads)..."

"$CMAKE" --build "$BUILD_DIR" -j "$NPROC" > "$BUILD_DIR/build.log" 2>&1 || {
    stop_spinner
    echo -e "  ${CROSS} Build failed"
    echo -e "     ${GRAY}See: examples/sdl_demo/build/build.log${NC}"
    exit 1
}

stop_spinner
ELAPSED=$((SECONDS - START_TIME))
echo -e "  ${CHECKMARK} Build complete ${GRAY}(${ELAPSED}s)${NC}"

# =============================================================================
# Output
# =============================================================================
if [[ "$OSTYPE" == msys* || "$OSTYPE" == mingw* || "$OSTYPE" == cygwin* ]]; then
    DEMO_EXE="$DEMO_DIR/bin/demo.exe"
else
    DEMO_EXE="$DEMO_DIR/bin/demo"
fi

if [[ -f "$DEMO_EXE" ]]; then
    SIZE=$(du -h "$DEMO_EXE" | cut -f1)
    echo ""
    echo -e "${GREEN}Output:${NC} examples/sdl_demo/bin/demo ${GRAY}(${SIZE})${NC}"
fi

echo ""
