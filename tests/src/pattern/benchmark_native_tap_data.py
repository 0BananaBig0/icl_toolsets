#!/usr/bin/env python3
"""Measure concrete TAP replay; prior fragmented-wide planner timed out."""
import benchmark_raw_tap_multiple_dr as multi


def scenarios():
    return multi.scenarios([(64,4,0,1,0),(16,16,0,1,0),(64,4,0,1,512),
                            (3,4,1024,1,0),(64,4,0,128,0)])


if __name__=='__main__':
    multi.ir.main(scenarios,multi.canonical,'MultipleDrTop','MultipleDrProbe',candidate_only=True)
