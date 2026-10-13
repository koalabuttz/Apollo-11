# SPDX-License-Identifier: MIT
# Flight image integration, phase 1 (Resident kernel lane; docs/FLIGHT-STORE.md).
# flight-store: every lane's routine tables in ABI 2 modules, packed into the
# REU store, with build/flight-store/report.json (fit, link failures,
# collisions). flight-host-differential: both missions from Fresh Start
# against yaAGC on the host, every native routine in lockstep, erasable
# memory compared each centisecond, logs and per-lane unregistered points in
# build/flight-store/differential/. Neither gates `test`: they measure the whole
# program while lanes are still filling it in. Need the listings
# (build/fixed-data/<mission>/assembly.lst) and the yaAGC objects.
.PHONY: flight-store flight-host-differential
flight-store: | build
	python3 tools/flight_store_build.py
flight-host-differential: build/agc_engine_hardware_dv.o | build
	python3 tools/flight_host_differential.py
