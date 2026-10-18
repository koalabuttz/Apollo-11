# SPDX-License-Identifier: MIT
# tools/flight_store_build.py's chunk packing (pack_chunks), on made-up
# routine sizes: no C64 tools needed.
.PHONY: test-flight-chunks
test: test-flight-chunks
test-flight-chunks:
	python3 tests/test_flight_chunks.py
