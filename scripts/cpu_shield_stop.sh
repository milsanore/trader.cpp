#!/usr/bin/env bash
# ------------------------------------------------------------
# reset CPU isolation, allow OS threads to run on all CPUs
# ------------------------------------------------------------
set -e

sudo cset shield --reset

echo "Reset CPU isolation, OS threads can run on all CPUs"
