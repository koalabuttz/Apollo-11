# SPDX-License-Identifier: MIT
# tools/reu_layout.py (the declared runtime REU regions and the write check
# flight_scenario_vice.py runs after each window), on made-up images.
.PHONY: test-reu-layout
test: test-reu-layout
test-reu-layout:
	python3 tests/test_reu_layout.py
