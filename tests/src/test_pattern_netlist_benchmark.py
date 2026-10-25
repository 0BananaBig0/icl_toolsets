#!/usr/bin/env python3
"""Bound the large-hierarchy and wide/narrow-contributor regression workload."""
import subprocess
import sys

if __name__ == '__main__':
    # Normal optimized runs take about a second; leave room for instrumented
    # builds and concurrent CI load while rejecting billions of padded visits.
    subprocess.run([sys.argv[1]], check=True, timeout=10)
