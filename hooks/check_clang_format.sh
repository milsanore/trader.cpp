#!/bin/bash

################################################################
# check_clang_format.sh
# checks for any formatting issues and errors out.
# does not modify files in-place.
################################################################

set -euo pipefail

echo ">> Checking for clang-format issues in src/ and tests/..."

CLANG_FORMAT_BIN="clang-format"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

files=$(find "$SCRIPT_DIR"/../src/ "$SCRIPT_DIR"/../tests/ \( -name '*.cpp' -o -name '*.hpp' -o -name '*.c' -o -name '*.h' \) | sort)
format_errors=0
for file in $files; do
    if [ -f "$file" ]; then
        if ! diff -q <($CLANG_FORMAT_BIN "$file") "$file" >/dev/null; then
            echo "clang-format issues found in file, exiting. file: [$file]"
            format_errors=1
            break
        fi
    fi
done

if [ $format_errors -ne 0 ]; then
    echo "ERROR: clang-format found unformatted code. Try \`make tidy\`"
    exit 1
fi
