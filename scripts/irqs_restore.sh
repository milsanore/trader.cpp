#!/bin/bash
# ------------------------------------------------------------
# Restore all IRQs to default CPU affinity (all CPUs)
# ------------------------------------------------------------

# Determine total number of CPUs
CPU_COUNT=$(nproc)

# Default mask: all CPUs (binary 111...1)
DEFAULT_MASK=$(printf "%X" $(( (1 << CPU_COUNT) - 1 )))

echo "Restoring all IRQs to default mask: $DEFAULT_MASK"

# Iterate through all IRQ directories
for irq_dir in /proc/irq/*; do
    [ -d "$irq_dir" ] || continue
    smp_file="$irq_dir/smp_affinity"

    [ -f "$smp_file" ] || continue

    # Restore default mask
    echo "$DEFAULT_MASK" > "$smp_file"
    echo "Restored IRQ $(basename $irq_dir) to all CPUs"
done
