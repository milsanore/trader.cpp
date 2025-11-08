#!/usr/bin/env python3
"""
HFT-ready Binance FIX API AZ inference script.

Steps:
1. Resolve hostname to IP(s)
2. Map IP(s) to AWS region using official AWS IP ranges
3. Optionally: run TCP ping/traceroute from this machine (or other AZs) to measure latency
4. Output structured results for multi-AZ comparison
"""

import socket
import requests
import ipaddress
import subprocess
import json
import sys

AWS_IP_RANGES_URL = "https://ip-ranges.amazonaws.com/ip-ranges.json"

def resolve_hostname(hostname):
    try:
        ips = socket.gethostbyname_ex(hostname)[2]
        print(f"{hostname} resolves to IPs: {ips}")
        return ips
    except Exception as e:
        print(f"ERROR: Could not resolve hostname: {e}")
        sys.exit(1)

def load_aws_prefixes():
    try:
        resp = requests.get(AWS_IP_RANGES_URL, timeout=10)
        resp.raise_for_status()
        data = resp.json()
    except Exception as e:
        print(f"ERROR: Could not download AWS IP ranges: {e}")
        sys.exit(1)
    prefixes = []
    for p in data.get("prefixes", []):
        try:
            net = ipaddress.ip_network(p["ip_prefix"])
            prefixes.append({
                "network": net,
                "region": p.get("region"),
                "service": p.get("service")
            })
        except Exception:
            continue
    return prefixes

def find_region_for_ip(ip_str, prefixes):
    ip_obj = ipaddress.ip_address(ip_str)
    for p in prefixes:
        if ip_obj in p["network"]:
            return p["region"], p["service"], str(p["network"])
    return None, None, None

def tcp_ping(ip, port=443, timeout=10):
    """Simple TCP ping to measure RTT (ms) using subprocess and 'timeout'"""
    try:
        import time
        start = time.time()
        sock = socket.create_connection((ip, port), timeout)
        sock.close()
        elapsed = (time.time() - start) * 1000  # ms
        return round(elapsed, 2)
    except Exception:
        return None

def traceroute(ip, max_hops=20):
    """Run traceroute to the target IP"""
    try:
        result = subprocess.run(
            ["traceroute", "-m", str(max_hops), ip],
            capture_output=True, text=True, timeout=35
        )
        return result.stdout
    except Exception as e:
        return f"Traceroute failed: {e}"

def main(hostname):
    ips = resolve_hostname(hostname)
    prefixes = load_aws_prefixes()
    results = []

    for ip in ips:
        region, service, network = find_region_for_ip(ip, prefixes)
        rtt = tcp_ping(ip)
        tr = traceroute(ip)
        results.append({
            "ip": ip,
            "aws_region": region,
            "aws_service": service,
            "aws_network": network,
            "tcp_ping_ms": rtt,
            "traceroute": tr
        })

    print("\n===== Results =====")
    print(json.dumps(results, indent=2))

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <hostname>")
        sys.exit(1)
    main(sys.argv[1])
