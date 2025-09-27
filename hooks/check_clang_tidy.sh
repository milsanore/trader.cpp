#!/bin/bash

################################################################
# check_clang_tidy.sh
# checks for any lint warnings and errors out.
# does not modify files in-place.
################################################################

set -euo pipefail

echo ">> Checking for clang-tidy warnings in src/..."

CLANG_TIDY_BIN="clang-tidy"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/../build/Debug"

if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
    echo "ERROR: compile_commands.json not found in $BUILD_DIR"
    echo "Run: cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -B $BUILD_DIR"
    exit 1
fi

files=$(find "$SCRIPT_DIR"/../src/ \( -name '*.cpp' -o -name '*.c' \) | sort)
tidy_errors=0
for file in $files; do
    echo "Running clang-tidy on $file"
    $CLANG_TIDY_BIN "$file" -p "$BUILD_DIR" > tidy_output.txt 2>&1 || true
    if grep -q "warning:" tidy_output.txt; then
        echo "clang-tidy warnings found in file, exiting. file: [$file] warnings:"
        cat tidy_output.txt
        tidy_errors=1
        break
    fi
done
rm tidy_output.txt

if [ $tidy_errors -ne 0 ]; then
    echo "ERROR: clang-tidy found warnings. Try \`make tidy\`"
    exit 1
fi
