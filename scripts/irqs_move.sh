#!/bin/bash
# ------------------------------------------------------------
# Move all IRQs (except NICs) to non-isolated CPUs dynamically
# ------------------------------------------------------------

set -euo pipefail
IFS=$'\n\t'

# List of NIC interfaces to protect (keep IRQs on isolated cores)
NIC_IFACES=("eth0" "eth1")  # modify as needed

# Determine total CPU count
CPU_COUNT=$(nproc)

# Set last isolated CPU index (adjust if your isolated CPUs differ)
# TODO: detect isolated CPUs dynamically (from /sys/devices/system/cpu/isolated)
ISOLATED_END=1   # e.g., CPUs 0-1 are isolated

# Build CPU mask for CPUs >= ISOLATED_END + 1
# Example: if CPU_COUNT=8, ISOLATED_END=1 -> mask = 11111100 = FC
# The mask is binary, then converted to hex
CPU_MASK=$(printf '%X' "$(( (1 << CPU_COUNT) - 1 - ((1 << (ISOLATED_END + 1)) - 1) ))")

echo "Detected CPU mask for IRQs: $CPU_MASK (CPUs >= $((ISOLATED_END + 1)))"

# ------------------------------------------------------------
# Helper function to check if an IRQ belongs to a NIC
# ------------------------------------------------------------
is_nic_irq() {
    local irq_dir="$1"
    local hint_file="$irq_dir/affinity_hint"
    [ -f "$hint_file" ] || return 1

    for nic in "${NIC_IFACES[@]}"; do
        if grep -q "$nic" "$hint_file" 2>/dev/null; then
            return 0  # It's a NIC IRQ
        fi
    done
    return 1  # Not a NIC IRQ
}

# ------------------------------------------------------------
# Iterate through all IRQ directories
# ------------------------------------------------------------
for irq_dir in /proc/irq/*; do
    [ -d "$irq_dir" ] || continue
    irq_num=$(basename "$irq_dir")
    smp_file="$irq_dir/smp_affinity"

    [ -f "$smp_file" ] || continue

    if is_nic_irq "$irq_dir"; then
        echo "Skipping NIC IRQ $irq_num"
        continue
    fi

    # Move IRQ to CPUs >= ISOLATED_END + 1
    echo "$CPU_MASK" > "$smp_file"
    echo "Moved IRQ $irq_num to CPUs >= $((ISOLATED_END + 1))"
done
