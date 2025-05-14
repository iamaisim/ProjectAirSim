"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

Demonstrates how to extract a simple coverage metric from the test bench.
"""
from projectairsim.test import TestBench

ideal_coverage_odd = 'testbench_complete_variations.jsonc'

# Import test bench
test_bench = TestBench.load_test_bench_from_file("testbench_example.csv", "testbench")
test_bench.estimate_coverage(ideal_coverage_odd)

