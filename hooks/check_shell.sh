#!/usr/bin/env bash

################################################################
# check_clang_tidy.sh
# checks for any lint warnings and errors out.
# does not modify files in-place.
# uses run-clang-tidy for parallel execution.
################################################################

set -euo pipefail

echo ">> Running shellcheck on all scripts"

while IFS= read -r file; do
    shellcheck "$file"
# find files that start with a sh/bash shebang, exclude build/ and .git/hooks folders
done < <(find . \( -type d -name build -o -path './.git/hooks' \) -prune -o -type f -exec grep -Iq '^#!.*sh' {} \; -print)
