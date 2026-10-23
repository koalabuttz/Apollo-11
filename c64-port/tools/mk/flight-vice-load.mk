# SPDX-License-Identifier: MIT
# tools/flight_vice_load.py's host reference for the load check (#426), on
# made-up module wrappers and results.
.PHONY: test-flight-vice-load
test: test-flight-vice-load
test-flight-vice-load:
	python3 tests/test_flight_vice_load.py
