# SPDX-License-Identifier: MIT
# The downlink word stream in flight (docs/FLIGHT-DOWNLINK.md).
# flight-downlink-check: the flight-restart-check scenarios with FLIGHT_DOWNRUPT
# (a DOWNRUPT every 20 ms of AGC time on both sides), the native channel 34/35
# words against yaAGC's in lockstep (and, unscaled, the free-running
# reference), each list whole and the stream restarted at the list's ID word
# after the GOJAM (tools/flight_downlink_check.py, build/flight-downlink/).
# flight-downlink-vice: DOWNRUPTs on the C64 flight store in VICE from native
# snapshots, the words checked against yaAGC (platform/c64/downlink_check.c,
# tools/flight_downlink_reference.c). Like flight-restart-check they need the
# listings and the yaAGC objects, and flight-downlink-vice the whole store;
# neither gates `test`.
.PHONY: flight-downlink-check flight-downlink-vice
flight-downlink-check: build/agc_engine_hardware_dv.o | build
	python3 tools/flight_downlink_check.py
flight-downlink-vice: flight-store build/agc_engine_hardware_dv.o
	python3 tools/flight_downlink_check.py --vice --c64-only
