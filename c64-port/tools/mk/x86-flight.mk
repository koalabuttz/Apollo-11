# SPDX-License-Identifier: MIT
# The native x86_64 flight program (docs/X86-FLIGHT.md). x86-flight builds
# build/x86-flight/x86-flight-<m> (compiled interpretive routines) and
# x86-flight-<m>-core (the interpreter core) over every lane's tables.
# x86-flight-check is its acceptance: the vehicle models' own checks
# (platform/x86/test_vehicle_models.c) and the radar's on both missions
# (platform/x86/test_radar_models.c), the landing radar's on the LM
# (platform/x86/test_landing_radar.c), the TC trap's
# (platform/x86/test_tc_trap.c), the hand controller's
# (platform/x86/test_hand_controller.c), the table kinds, the RESTART lamp's
# (platform/x86/test_restart_lamp.c, built by x86_flight_check.py), then P00 on both
# missions and the LM DAP hold in lockstep against yaAGC (clock frozen, the
# x86 program's schedule), in both modes. x86-flight-mct re-measures the
# routine costs (docs/flight-routine-mct.json) from the host differential.
# x86-flight-scenarios flies the host differential's flight scenarios on the
# x86 program with its vehicle models and compares each with the harness on
# the work clock (tools/x86_flight_scenarios.py; not in CI).
# All need the listings (build/fixed-data/<mission>/assembly.lst).
.PHONY: x86-flight x86-flight-check x86-flight-mct x86-flight-scenarios
x86-flight: | build
	python3 tools/x86_flight_build.py
x86-flight-check: build/agc_engine_hardware_dv.o | build
	$(CC) -std=c99 -O2 -g -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Iplatform/x86 \
	  platform/x86/test_vehicle_models.c -lm -o build/test-vehicle-models
	./build/test-vehicle-models
	for m in LM CM; do \
	  $(CC) -std=c99 -O2 -g -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Iplatform/x86 \
	    -DRK_$$m=1 platform/x86/test_radar_models.c -lm -o build/test-radar-models-$$m && \
	  ./build/test-radar-models-$$m || exit 1; \
	done
	$(CC) -std=c99 -O2 -g -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Iplatform/x86 \
	  -DRK_LM=1 platform/x86/test_landing_radar.c -lm -o build/test-landing-radar
	./build/test-landing-radar
	$(CC) -std=c99 -O2 -g -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Iplatform/x86 \
	  platform/x86/test_tc_trap.c -o build/test-tc-trap
	./build/test-tc-trap
	$(CC) -std=c99 -O2 -g -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Iplatform/x86 \
	  platform/x86/test_night_watchman.c -o build/test-night-watchman
	./build/test-night-watchman
	$(CC) -std=c99 -O2 -g -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Iplatform/x86 \
	  platform/x86/test_hand_controller.c -o build/test-hand-controller
	./build/test-hand-controller
	python3 tools/flight_store_build.py --host-only
	python3 tools/flight_table_kinds.py
	python3 tools/x86_flight_check.py --skip-index
x86-flight-mct: build/agc_engine_hardware_dv.o | build
	python3 tools/x86_flight_mct.py
x86-flight-scenarios: build/agc_engine_hardware_dv.o | build
	python3 tools/x86_flight_scenarios.py
