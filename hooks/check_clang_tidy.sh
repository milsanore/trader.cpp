#!/bin/bash

################################################################
# check_clang_tidy.sh
# checks for any lint warnings and errors out.
# does not modify files in-place.
# uses run-clang-tidy for parallel execution.
################################################################

set -euo pipefail

echo ">> Checking for clang-tidy warnings in [ src/ tests/ benchmarks/ ]"

CLANG_TIDY_BIN="run-clang-tidy-18"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/../build/Debug"

if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
    echo "ERROR: compile_commands.json not found in $BUILD_DIR"
    echo "Run: cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -B $BUILD_DIR"
    exit 1
fi

# Run run-clang-tidy in parallel on src/ , tests/ , and benchmarks/ directories
# no -fix flag to just report warnings/errors
output=$($CLANG_TIDY_BIN -p "$BUILD_DIR" -j "$(nproc)" -header-filter='src/.*|tests/.*|benchmarks/.*' src tests benchmarks 2>&1) || true

echo "$output"

# Check if output contains any warnings or errors
if echo "$output" | grep -qE "warning:|error:"; then
    echo "ERROR: clang-tidy found warnings or errors. Try \`make tidy\`"
    exit 1
fi

echo "..."
echo "No clang-tidy warnings found."
