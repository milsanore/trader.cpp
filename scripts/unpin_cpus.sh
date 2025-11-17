#!/usr/bin/env bash

# -------------------------------------------------------
# CPU isolation cleanup script (supports cgroup v1 & v2).
# Undoes the actions of pin_cpus.sh (see for additional context).
# -------------------------------------------------------

set -euo pipefail

# Validate required environment variables
: "${CPU_SET_NAME:?Environment variable CPU_SET_NAME is required}"
echo "CPU set cleanup starting..."
echo "CPU_SET_NAME [$CPU_SET_NAME]"

# Check for root
if [ "$EUID" -ne 0 ]; then
    echo "this script must be run as root."
    echo "try: sudo $0"
    exit 1
fi

# Build path based on cgroup version
if [[ -f /sys/fs/cgroup/cgroup.controllers ]]; then
    CGROUP_VERSION=2
    BASE_CGROUP="/sys/fs/cgroup"
else
    CGROUP_VERSION=1
    BASE_CGROUP="/sys/fs/cgroup/cpuset"
fi
echo "detected cgroup version. value [$CGROUP_VERSION]"
GROUP_PATH="$BASE_CGROUP/$CPU_SET_NAME"

# Check existence
if [[ ! -d "$GROUP_PATH" ]]; then
    echo "nothing to clean — cpuset does not exist. path [$GROUP_PATH]"
    exit 0
fi
echo "found cpuset. path [$GROUP_PATH]"

# Ensure cpuset is empty
PROCS_FILE="$GROUP_PATH/cgroup.procs"
if [[ ! -f "$PROCS_FILE" ]]; then
    echo "ERROR: cgroup.procs file missing — unexpected layout!"
    exit 1
fi

if [[ -s "$PROCS_FILE" ]]; then
    echo "ERROR: cpuset is not empty — processes still inside:"
    cat "$PROCS_FILE"
    echo "please stop or move these processes before cleanup."
    exit 1
fi

# Delete cpuset directory
echo "removing cpuset directory. path [$GROUP_PATH]"
rmdir "$GROUP_PATH"

echo "cleanup complete."
