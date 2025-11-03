#!/usr/bin/env bash
# ------------------------------------------------------------
# move all OS threads off of trader CPUs
# ------------------------------------------------------------
set -e

sudo cset shield --cpu 0-1 --kthread=on

echo "Isolated CPUs 0 and 1 for the trader app, all OS threads will be moved"
