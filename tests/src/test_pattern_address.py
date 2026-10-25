#!/usr/bin/env python3
"""Bound address resolution for wide shared and unrelated parent buses."""
import subprocess
import sys

if __name__ == '__main__':
    # Optimized and sanitizer suites should be well below ten seconds. The
    # regression repeatedly materialized 1M unrelated bits for 10,000 small banks.
    subprocess.run([sys.argv[1]], check=True, timeout=10)
