#!/usr/bin/env bash

# ----------------------------------------------------------
# CPU isolation launcher for Linux (cgroup v1 & v2)
#
# How it works:
# 1. Root required: The script checks for root because writing to /sys/fs/cgroup requires privileges.
# 2. Validation: Ensures CPU_SET_NAME and CPU_SET_RANGE exist.
# 3. Cgroup detection: Automatically handles v1 and v2.
# 4. CPU and memory configuration: Writes cpuset.cpus and cpuset.mems.
# 5. Associating the application
# ----------------------------------------------------------

set -euo pipefail

# validate required environment variables
: "${CPU_SET_NAME:?Environment variable CPU_SET_NAME is required}"
: "${CPU_SET_RANGE:?Environment variable CPU_SET_RANGE is required}"
echo "CPU isolation launcher starting..."
echo "CPU_SET_NAME [$CPU_SET_NAME]"
echo "CPU_SET_RANGE [$CPU_SET_RANGE]"

# check for root
if [ "$EUID" -ne 0 ]; then
    echo "this script must be run as root."
    echo "try: sudo $0"
    exit 1
fi

if ! [[ $CPU_SET_RANGE =~ ^[0-9,-]+$ ]]; then
    echo "invalid CPU_SET_RANGE format. value [$CPU_SET_RANGE]"
    exit 1
fi

# --- NUMA check ---
expand_cpu_range() {
    local range=$1
    local cpus=()
    IFS=',' read -ra parts <<< "$range"
    for part in "${parts[@]}"; do
        if [[ $part == *-* ]]; then
            IFS='-' read -r start end <<< "$part"
            for ((i=start; i<=end; i++)); do
                cpus+=("$i")
            done
        else
            cpus+=("$part")
        fi
    done
    echo "${cpus[@]}"
}
# determine NUMA nodes used by requested CPUs
nodes_used=$(for cpu in $(expand_cpu_range "$CPU_SET_RANGE"); do
    for node_dir in /sys/devices/system/node/node*; do
        cpulist_file="$node_dir/cpulist"
        [[ -f $cpulist_file ]] || continue
        if grep -qE "(^|,)$cpu($|,|-)" "$cpulist_file"; then
            basename "$node_dir" | sed 's/node//'
        fi
    done
done | sort -u)
if [[ -z "$nodes_used" ]]; then
    echo "error: could not determine NUMA nodes. CPUs [$CPU_SET_RANGE]"
    exit 1
fi
if [[ $(wc -w <<< "$nodes_used") -gt 1 ]]; then
    echo "error: requested CPUs span multiple NUMA nodes. CPUs [$CPU_SET_RANGE] nodes [$nodes_used]"
    exit 1
fi
echo "requested CPUs are within NUMA node. CPUs [$CPU_SET_RANGE] node [$nodes_used]"
# --- end NUMA check ---

# build path based on cgroup version
if [[ -f /sys/fs/cgroup/cgroup.controllers ]]; then
    CGROUP_VERSION=2
    BASE_CGROUP="/sys/fs/cgroup"
else
    CGROUP_VERSION=1
    BASE_CGROUP="/sys/fs/cgroup/cpuset"
fi
echo "detected cgroup version. value [$CGROUP_VERSION]"
GROUP_PATH="$BASE_CGROUP/$CPU_SET_NAME"

# create cpuset directory if it doesn't exist
if [[ ! -d "$GROUP_PATH" ]]; then
    mkdir -p "$GROUP_PATH"
    echo "created cpuset directory. path [$GROUP_PATH]"
fi

# write CPUs
CPU_FILE="$GROUP_PATH/cpuset.cpus"
echo "$CPU_SET_RANGE" > "$CPU_FILE"
echo "wrote CPUs to file. path [$CPU_FILE]"

# write memory nodes (required for cpuset)
MEM_FILE="$GROUP_PATH/cpuset.mems"
if [[ ! -f "$MEM_FILE" ]]; then
    echo "$nodes_used" > "$MEM_FILE"
fi
echo "set memory nodes in file. path [$MEM_FILE]"

# add current process to the cpuset
PROCS_FILE="$GROUP_PATH/cgroup.procs"
# NB: $$ is the PID of the script itself
echo "$$" > "$PROCS_FILE"
echo "added process to cpuset. PID [$$] path [$PROCS_FILE]"
echo "launching application in isolated cpuset..."
# NB: when we exec "$@", the application replaces the script and inherits the cgroup
"$@"
# print application return so that the script doesn't obfuscate it
ret=$?
echo "Application exited with code $ret"
exit $ret
