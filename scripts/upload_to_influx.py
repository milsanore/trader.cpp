#!/usr/bin/env python3
import json, os, requests, time

# GitHub Actions environment variables for commit info
commit = os.getenv("GITHUB_SHA", "")[:8]
branch = os.getenv("GITHUB_REF_NAME", "unknown")
source = os.getenv("BENCH_SOURCE", "unknown")

# InfluxDB config from GitHub secrets
INFLUX_URL = os.environ["INFLUX_URL"]
INFLUX_ORG = os.environ["INFLUX_ORG"]
INFLUX_BUCKET = os.environ["INFLUX_BUCKET"]
INFLUX_TOKEN = os.environ["INFLUX_TOKEN"]

timestamp = int(time.time() * 1e9)  # nanoseconds

with open("bench_results.json") as f:
    data = json.load(f)

lines = []
for bm in data.get("benchmarks", []):
    name = bm["name"].replace(" ", "_").replace("/", "_")
    real_time = bm.get("real_time", 0.0)
    cpu_time = bm.get("cpu_time", 0.0)
    items_per_sec = bm.get("items_per_second", 0.0)
    iterations = bm.get("iterations", 0)

    # Line Protocol with 'source' as a tag
    line = (
        f"benchmarks,name={name},commit={commit},branch={branch},source={source} "
        f"real_time={real_time},cpu_time={cpu_time},items_per_sec={items_per_sec},iterations={iterations} "
        f"{timestamp}"
    )
    lines.append(line)

body = "\n".join(lines)

resp = requests.post(
    f"{INFLUX_URL}/api/v2/write?org={INFLUX_ORG}&bucket={INFLUX_BUCKET}&precision=ns",
    headers={
        "Authorization": f"Token {INFLUX_TOKEN}",
        "Content-Type": "text/plain; charset=utf-8",
    },
    data=body,
)

if resp.status_code != 204:
    print("❌ Failed to write to InfluxDB:", resp.text)
    resp.raise_for_status()
else:
    print(f"✅ Uploaded {len(lines)} benchmarks to InfluxDB with source={source}")
