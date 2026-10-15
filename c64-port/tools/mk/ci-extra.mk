# SPDX-License-Identifier: MIT
# Checks that PR CI runs although they are outside the test-, sanitize- and
# c64- groups (tools/ci_affected.py). A PR runs each one when a changed file
# is in its closure, or in its ci-watch-<target> list when the closure is too
# wide to select on (the interp-flight-*-vice checks need the whole flight
# store, and interp-core-differential every compiled milestone, so they watch
# the interpreter's own files instead; x86-flight-check links every source,
# so it watches them all). Full runs run them all.
.PHONY: ci-extra-host ci-extra-c64 ci-watch-interp-flight-vice ci-watch-interp-flight-flowed-vice \
  ci-watch-interp-flight-flowed-vice-1 ci-watch-interp-flight-flowed-vice-2 ci-watch-interp-flight-flowed-vice-3 \
  ci-watch-interp-flight-flowed-vice-4 \
  ci-watch-interp-flight-p63-vice ci-watch-interp-core-differential ci-watch-x86-flight-check \
  ci-watch-flight-restart-check ci-watch-flight-restart-vice ci-watch-flight-gojam-vice ci-watch-flight-downlink-check \
  ci-watch-flight-downlink-vice overlay-v2-check ci-watch-overlay-v2-check
ci-extra-host: interp-runtime-check interp-core-fuzz interp-core-differential x86-flight-check flight-restart-check \
  flight-downlink-check overlay-v2-check
ci-extra-c64: vice-interp-core interp-flight-vice interp-flight-flowed-vice-1 interp-flight-flowed-vice-2 interp-flight-flowed-vice-3 \
  interp-flight-flowed-vice-4 interp-flight-p63-vice flight-restart-vice flight-gojam-vice flight-downlink-vice
CI_EXTRA_INTERP := src/lm_interp_core.c src/cm_interp_core.c src/lm_interp_orders.c src/lm_interp_steps.c \
  src/cm_interp_orders.c src/cm_interp_steps.c include/interp_runtime_tables.h
ci-watch-interp-flight-vice: $(CI_EXTRA_INTERP) tools/interp_flight_test.py platform/c64/interp_flight_check.c \
  tools/flight_store_build.py
ci-watch-interp-flight-flowed-vice ci-watch-interp-flight-flowed-vice-1 ci-watch-interp-flight-flowed-vice-2 ci-watch-interp-flight-flowed-vice-3 \
  ci-watch-interp-flight-flowed-vice-4: $(CI_EXTRA_INTERP) src/lm_interp_runtime.c tools/interp_flight_test.py \
  tools/interp_flight_flowed.c tools/interp_flight_verify.c platform/c64/interp_flight_check.c tools/flight_store_build.py
ci-watch-interp-flight-p63-vice: $(CI_EXTRA_INTERP) tools/interp_flight_test.py tools/interp_flight_capture.c \
  tools/scenarios/luminary099-p63-ignition.core platform/c64/interp_flight_check.c tools/flight_store_build.py
ci-watch-interp-core-differential: $(CI_EXTRA_INTERP) tools/interp_runtime_differential.py \
  tools/interp_runtime_target.c tools/interp_routine_reference.c
ci-watch-x86-flight-check: $(wildcard src/*.c src/*.inc include/*.h platform/x86/*) tools/flight_store_build.py \
  tools/flight_table_kinds.py tools/x86_flight_build.py tools/x86_flight_check.py tools/x86_flight_lockstep.c \
  docs/flight-routine-mct.json
# flight-restart-check and -vice link every source too: they watch the
# restart's own files and the harness.
CI_EXTRA_RESTART := src/lm_fresh_start.c src/cm_fresh_start.c src/lm_restart_routines.c src/lm_restart_select.c \
  src/cm_restart_routines.c src/cm_restart_select.c src/lm_phase_table.c src/cm_phase_table.c tools/flight_gojam.h tools/flight_downlink.h \
  tools/flight_host_run.c tools/flight_host_differential.py tools/flight_restart_check.py
ci-watch-flight-restart-check: $(CI_EXTRA_RESTART)
ci-watch-flight-restart-vice: $(CI_EXTRA_RESTART) platform/c64/restart_check.c tools/flight_restart_reference.c
ci-watch-flight-gojam-vice: $(CI_EXTRA_RESTART) platform/c64/restart_check.c tools/flight_restart_reference.c \
  platform/c64/resident_gojam_module.c src/overlay_v2.c include/overlay_v2.h
# flight-downlink-check and -vice run the restart scenarios: they watch the
# restart's files, the downlink's own and downtelemetry's.
CI_EXTRA_DOWNLINK := $(CI_EXTRA_RESTART) src/lm_downtelemetry.c src/cm_downtelemetry.c src/lm_downtelemetry_data.inc \
  src/cm_downtelemetry_data.inc tools/flight_downlink.h tools/flight_downlink_check.py
ci-watch-flight-downlink-check: $(CI_EXTRA_DOWNLINK)
ci-watch-flight-downlink-vice: $(CI_EXTRA_DOWNLINK) platform/c64/downlink_check.c tools/flight_downlink_reference.c
# The overlay ABI 2 host test (tools/overlay_v2.mk, tests/test_overlay_v2.c),
# plain and sanitized: the loader, cache, seams, faults and both restarts
# every flight build links. Its allocation-plan step is left out: it
# compares a planning report, not the runtime.
OV2_CHECK_STORES := build/overlay-v2/selftest.reu build/overlay-v2/selftest-pinned-task.reu
overlay-v2-check:
	$(MAKE) -f tools/overlay_v2.mk $(OV2_CHECK_STORES) build/overlay-v2/test build/overlay-v2/test-sanitize
	./build/overlay-v2/test $(OV2_CHECK_STORES)
	ASAN_OPTIONS=detect_leaks=0 ./build/overlay-v2/test-sanitize $(OV2_CHECK_STORES)
ci-watch-overlay-v2-check: src/overlay_v2.c include/overlay_v2.h src/overlay_reu.c include/overlay_reu.h \
  src/overlay_runtime.c include/overlay_runtime.h tests/test_overlay_v2.c tools/overlay_v2_pack.py tools/overlay_v2.mk
