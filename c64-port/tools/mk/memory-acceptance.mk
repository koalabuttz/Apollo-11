# SPDX-License-Identifier: MIT
# tools/memory_acceptance.py (each program's memory_and_overlay_validation
# document) on made-up runs, linker scripts and stores.
.PHONY: test-memory-acceptance
test: test-memory-acceptance
test-memory-acceptance:
	python3 tests/test_memory_acceptance.py
