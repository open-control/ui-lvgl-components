#!/bin/bash
# Watch and rebuild SDL demo on file changes
# Usage: ./watch_demo.sh [--install-watchexec]
set -e

# =============================================================================
# Colors
# =============================================================================
RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'
CYAN='\033[0;36m'; GRAY='\033[0;90m'; BOLD='\033[1m'; NC='\033[0m'

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
# Paths to watch
# =============================================================================
WATCH_PATHS=(
    "src"
    "include"
    "examples/sdl_demo/src"
)

# Executable
if [[ "$OSTYPE" == msys* || "$OSTYPE" == mingw* || "$OSTYPE" == cygwin* ]]; then
    DEMO_EXE="examples/sdl_demo/bin/demo.exe"
    IS_WINDOWS=true
else
    DEMO_EXE="examples/sdl_demo/bin/demo"
    IS_WINDOWS=false
fi

DEMO_PID=""

# =============================================================================
# Cleanup on exit
# =============================================================================
cleanup() {
    echo -e "\n${YELLOW}Stopping...${NC}"
    kill_demo
    exit 0
}
trap cleanup SIGINT SIGTERM

# =============================================================================
# Kill running demo
# =============================================================================
kill_demo() {
    if [[ -n "$DEMO_PID" ]]; then
        kill "$DEMO_PID" 2>/dev/null || true
        wait "$DEMO_PID" 2>/dev/null || true
        DEMO_PID=""
    fi
    # Also kill any orphan demo processes
    if $IS_WINDOWS; then
        taskkill //F //IM demo.exe 2>/dev/null || true
    else
        pkill -f "examples/sdl_demo/bin/demo" 2>/dev/null || true
    fi
}

# =============================================================================
# Build and run
# =============================================================================
build_and_run() {
    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${BOLD}Rebuilding...${NC}"

    kill_demo

    if ./build_demo.sh; then
        echo -e "${GREEN}Starting demo...${NC}"
        "$DEMO_EXE" &
        DEMO_PID=$!
        echo -e "${GRAY}PID: $DEMO_PID${NC}"
    else
        echo -e "${RED}Build failed!${NC}"
    fi

    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${GRAY}Watching for changes... (Ctrl+C to stop)${NC}"
    echo ""
}

# =============================================================================
# Check for watchexec
# =============================================================================
if command -v watchexec &> /dev/null; then
    echo -e "${GREEN}Using watchexec for file watching${NC}"
    echo -e "${GRAY}Watching: ${WATCH_PATHS[*]}${NC}"
    echo -e "${GRAY}Press Ctrl+C to stop${NC}"
    echo ""

    # Build extensions to watch
    EXTS="cpp,hpp,c,h"

    # Use watchexec with restart mode
    # --restart: kill and restart the command on changes
    # --stop-signal SIGKILL: force kill on Windows
    watchexec \
        --restart \
        --stop-signal SIGKILL \
        --exts "$EXTS" \
        --watch src \
        --watch include \
        --watch examples/sdl_demo/src \
        --debounce 500 \
        -- ./build_demo.sh "&&" "$DEMO_EXE"
else
    # =============================================================================
    # Fallback: Simple polling (works everywhere)
    # =============================================================================
    echo -e "${YELLOW}watchexec not found, using polling (slower)${NC}"
    echo -e "${GRAY}Install watchexec for better performance:${NC}"
    echo -e "${GRAY}  Windows: winget install watchexec${NC}"
    echo -e "${GRAY}  Or: cargo install watchexec-cli${NC}"
    echo ""

    POLL_INTERVAL=2  # seconds

    # Get initial checksums
    get_checksum() {
        find "${WATCH_PATHS[@]}" -name "*.cpp" -o -name "*.hpp" -o -name "*.c" -o -name "*.h" 2>/dev/null | \
            xargs stat --format="%Y %n" 2>/dev/null | sort | md5sum | cut -d' ' -f1
    }

    LAST_CHECKSUM=$(get_checksum)

    # Initial build
    build_and_run

    # Poll loop
    while true; do
        sleep $POLL_INTERVAL

        CURRENT_CHECKSUM=$(get_checksum)

        if [[ "$CURRENT_CHECKSUM" != "$LAST_CHECKSUM" ]]; then
            LAST_CHECKSUM="$CURRENT_CHECKSUM"
            build_and_run
        fi
    done
fi
