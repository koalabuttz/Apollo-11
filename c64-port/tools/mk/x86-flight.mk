# SPDX-License-Identifier: MIT
# The native x86_64 flight program (docs/X86-FLIGHT.md). x86-flight builds
# build/x86-flight/x86-flight-<m> (compiled interpretive routines) and
# x86-flight-<m>-core (the interpreter core) over every lane's tables.
# x86-flight-check is its acceptance: the table kinds, then P00 on both
# missions and the LM DAP hold in lockstep against yaAGC (clock frozen, the
# x86 program's schedule), in both modes. x86-flight-mct re-measures the
# routine costs (docs/flight-routine-mct.json) from the host differential.
# All need the listings (build/fixed-data/<mission>/assembly.lst).
.PHONY: x86-flight x86-flight-check x86-flight-mct
x86-flight: | build
	python3 tools/x86_flight_build.py
x86-flight-check: build/agc_engine_hardware_dv.o | build
	python3 tools/flight_store_build.py --host-only
	python3 tools/flight_table_kinds.py
	python3 tools/x86_flight_check.py --skip-index
x86-flight-mct: build/agc_engine_hardware_dv.o | build
	python3 tools/x86_flight_mct.py
