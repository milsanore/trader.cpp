#!/usr/bin/env bash

# --------------------------------------------------------
# IRQ isolation script
# Moves all IRQs off of the specified isolated CPUs
#
# How it works:
# 1. CPU range → bitmask
#   - Converts something like 0-3,5 → 0b00101111 → 0x2F
# 2. Iterates /proc/irq/*/smp_affinity
#   - Reads current mask
#   - Clears bits for isolated CPUs
#   - Writes new mask back
# 3. Safe
#   - Only writes if a change is needed
#   - Skips files that aren't writable
#
# How to undo: reboot
# --------------------------------------------------------

set -euo pipefail

: "${CPU_SET_RANGE:?Environment variable CPU_SET_RANGE is required}"
echo "IRQ isolation starting..."
echo "CPU_SET_RANGE [$CPU_SET_RANGE]"

# check for root
if [ "$EUID" -ne 0 ]; then
    echo "this script must be run as root."
    echo "try: sudo $0"
    exit 1
fi

# --- Convert CPU range to bitmask ---
cpu_to_mask() {
    # converts something like 0-3,5 to a hex bitmask string
    local cpus=()
    IFS=',' read -ra parts <<< "$1"
    for part in "${parts[@]}"; do
        if [[ $part =~ ^([0-9]+)-([0-9]+)$ ]]; then
            for ((i=BASH_REMATCH[1]; i<=BASH_REMATCH[2]; i++)); do
                cpus+=("$i")
            done
        else
            cpus+=("$part")
        fi
    done

    # build bitmask
    local mask=0
    for cpu in "${cpus[@]}"; do
        mask=$((mask | (1 << cpu)))
    done
    printf "%x" "$mask"
}
ISOLATED_MASK=$(cpu_to_mask "$CPU_SET_RANGE")
echo "isolated CPU mask: 0x[$ISOLATED_MASK]"

# iterate over all IRQs
for irq in /proc/irq/*/smp_affinity; do
    # skip non-files
    [[ -f $irq ]] || continue 
    # Check if writable
    if [[ ! -w "$irq" ]]; then
        echo "skipping IRQ, smp_affinity not writable. IRQ [$(basename "$(dirname "$irq")")]"
        continue
    fi
    # read current mask
    current=$(cat "$irq")
    # convert hex to decimal
    current_dec=$((16#$current))
    # remove isolated CPUs
    new_dec=$((current_dec & ~0x$ISOLATED_MASK))
    # calculate new mask
    new_hex=$(printf "%x" "$new_dec")
    # only write if different
    if [[ "$current" != "$new_hex" ]]; then
        echo "moving IRQ off isolated CPUs. IRQ [$(basename "$(dirname "$irq")")] old [$current] new [$new_hex]"
        echo "$new_hex" > "$irq"
    fi
done

echo "IRQ isolation complete."
