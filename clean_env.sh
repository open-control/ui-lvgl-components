#!/bin/bash
# Clean build environment
set -e

# =============================================================================
# Colors
# =============================================================================
RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'
CYAN='\033[0;36m'; GRAY='\033[0;90m'; BOLD='\033[1m'; NC='\033[0m'
CHECKMARK="${GREEN}✓${NC}"

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

echo ""
echo -e "${BOLD}Cleaning Environment${NC}"
echo -e "${GRAY}─────────────────────────────────────────${NC}"

# =============================================================================
# Parse arguments (default: clean everything)
# =============================================================================
CLEAN_TOOLS=true
CLEAN_PIO=true

for arg in "$@"; do
    case "$arg" in
        --build-only) CLEAN_TOOLS=false; CLEAN_PIO=false ;;
        --help|-h)
            echo "Usage: ./clean_env.sh [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  (none)        Clean everything (build + tools + pio)"
            echo "  --build-only  Clean build artifacts only"
            echo ""
            exit 0
            ;;
    esac
done

# =============================================================================
# Clean build artifacts (always)
# =============================================================================
DEMO_DIR="$PROJECT_ROOT/examples/sdl_demo"

rm -rf "$DEMO_DIR/build" 2>/dev/null && echo -e "  ${CHECKMARK} Removed build/"
rm -rf "$DEMO_DIR/bin" 2>/dev/null && echo -e "  ${CHECKMARK} Removed bin/"
rm -rf "$DEMO_DIR/deps" 2>/dev/null && echo -e "  ${CHECKMARK} Removed deps/"
rm -f "$PROJECT_ROOT/.pio_env" 2>/dev/null && echo -e "  ${CHECKMARK} Removed .pio_env"

# =============================================================================
# Clean tools (optional)
# =============================================================================
if $CLEAN_TOOLS; then
    # Delete platform-specific tool directories (NOT zig-toolchain.cmake)
    # Zig format: zig-{arch}-{os}-{version} (e.g., zig-x86_64-windows-0.15.2)
    rm -rf "$PROJECT_ROOT"/tools/zig-x86_64-* 2>/dev/null
    rm -rf "$PROJECT_ROOT"/tools/zig-aarch64-* 2>/dev/null
    rm -rf "$PROJECT_ROOT"/tools/cmake-* 2>/dev/null
    rm -rf "$PROJECT_ROOT"/tools/ninja-* 2>/dev/null
    rm -f "$PROJECT_ROOT"/tools/zig-cc* 2>/dev/null
    rm -f "$PROJECT_ROOT"/tools/zig-c++* 2>/dev/null
    rm -f "$PROJECT_ROOT"/tools/zig-ar* 2>/dev/null
    echo -e "  ${CHECKMARK} Removed tools/ (Zig, CMake, Ninja)"
fi

# =============================================================================
# Clean PlatformIO (optional)
# =============================================================================
if $CLEAN_PIO; then
    rm -rf "$PROJECT_ROOT/.pio" 2>/dev/null && echo -e "  ${CHECKMARK} Removed .pio/"
fi

echo ""
echo -e "${GREEN}Clean complete${NC}"
echo ""
