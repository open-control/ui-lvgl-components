#!/bin/bash
# Run SDL demo (no terminal output, just execute)

# Find project root
dir="$PWD"
while [[ "$dir" != "/" && ! -f "$dir/platformio.ini" ]]; do dir="$(dirname "$dir")"; done
cd "$dir"

# Executable path
if [[ "$OSTYPE" == msys* || "$OSTYPE" == mingw* || "$OSTYPE" == cygwin* ]]; then
    EXE="examples/sdl_demo/bin/demo.exe"
else
    EXE="examples/sdl_demo/bin/demo"
fi

# Build if needed, then run
[[ ! -f "$EXE" ]] && ./build_demo.sh
exec "$EXE"
