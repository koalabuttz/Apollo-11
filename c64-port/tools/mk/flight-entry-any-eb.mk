# SPDX-License-Identifier: MIT
# Flight parts' shared entry keeps each any-EB key's basic_sane_any_eb
# (tests/test_flight_entry_any_eb.py, on the generated sources): no C64
# tools needed.
.PHONY: test-flight-entry-any-eb
test: test-flight-entry-any-eb
test-flight-entry-any-eb:
	python3 tests/test_flight_entry_any_eb.py
