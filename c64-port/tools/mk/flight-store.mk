# SPDX-License-Identifier: MIT
# Flight image integration, phase 1 (Resident kernel lane; docs/FLIGHT-STORE.md).
# flight-store: every lane's routine tables in ABI 2 modules, packed into the
# REU store, with build/flight-store/report.json (fit, link failures,
# collisions). flight-host-differential: both missions from Fresh Start
# against yaAGC on the host, every native routine in lockstep, erasable
# memory compared each centisecond, logs and per-lane unregistered points in
# build/flight-store/differential/. flight-store-vice: the flight resident
# boots on the store in VICE, resolves every record and times each module's
# load (build/flight-store/vice-load.json). flight_store_build.py skips the
# build when its inputs and last outputs are unchanged (build/flight-store/
# .stamp-<missions>; --force rebuilds). tools/native_range_keys.py then checks
# that every native_ranges return point has a key in its mission's store.
# None gates `test`: they measure the whole program while lanes are still
# filling it in. Need the listings
# (build/fixed-data/<mission>/assembly.lst) and the yaAGC objects.
.PHONY: flight-store flight-store-lm flight-host-differential flight-store-vice flight-disk flight-disk-vice
flight-store: | build
	python3 tools/flight_store_build.py
	python3 tools/native_range_keys.py
# flight-store-lm: the LM store alone, half the build, for checks that boot
# only lm.reu (the LM interp-flight-*-vice targets).
flight-store-lm: | build
	python3 tools/flight_store_build.py --missions lm
flight-host-differential: build/agc_engine_hardware_dv.o | build
	python3 tools/flight_host_differential.py
flight-store-vice: flight-store
	python3 tools/flight_vice_load.py
# flight-disk: the 1581 disk sets that boot each store on a stock C64 with
# a 2 MB REU, and the hardware check disk (docs/RUN-ON-C64.md), in
# build/flight-disk/; the check runs the host flight run. flight-disk-vice
# boots them in VICE from the disks alone (true drive emulation, the REU
# starting zero) and times the load (build/flight-disk/vice.json).
flight-disk: flight-store build/agc_engine_hardware_dv.o
	python3 tools/flight_disk.py
flight-disk-vice: flight-store build/agc_engine_hardware_dv.o
	python3 tools/flight_disk.py --vice
