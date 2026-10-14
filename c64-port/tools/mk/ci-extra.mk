# SPDX-License-Identifier: MIT
# Checks that PR CI runs although they are outside the test-, sanitize- and
# c64- groups (tools/ci_affected.py). A PR runs each one when a changed file
# is in its closure, or in its ci-watch-<target> list when the closure is too
# wide to select on (the interp-flight-*-vice checks need the whole flight
# store, and interp-core-differential every compiled milestone, so they watch
# the interpreter's own files instead; x86-flight-check links every source,
# so it watches them all). Full runs run them all.
.PHONY: ci-extra-host ci-extra-c64 ci-watch-interp-flight-vice ci-watch-interp-flight-flowed-vice \
  ci-watch-interp-flight-p63-vice ci-watch-interp-core-differential ci-watch-x86-flight-check
ci-extra-host: interp-runtime-check interp-core-fuzz interp-core-differential x86-flight-check
ci-extra-c64: vice-interp-core interp-flight-vice interp-flight-flowed-vice interp-flight-p63-vice
CI_EXTRA_INTERP := src/lm_interp_core.c src/cm_interp_core.c src/lm_interp_orders.c src/lm_interp_steps.c \
  src/cm_interp_orders.c src/cm_interp_steps.c include/interp_runtime_tables.h
ci-watch-interp-flight-vice: $(CI_EXTRA_INTERP) tools/interp_flight_test.py platform/c64/interp_flight_check.c \
  tools/flight_store_build.py
ci-watch-interp-flight-flowed-vice: $(CI_EXTRA_INTERP) src/lm_interp_runtime.c tools/interp_flight_test.py \
  tools/interp_flight_flowed.c tools/interp_flight_verify.c platform/c64/interp_flight_check.c tools/flight_store_build.py
ci-watch-interp-flight-p63-vice: $(CI_EXTRA_INTERP) tools/interp_flight_test.py tools/interp_flight_capture.c \
  tools/scenarios/luminary099-p63-ignition.core platform/c64/interp_flight_check.c tools/flight_store_build.py
ci-watch-interp-core-differential: $(CI_EXTRA_INTERP) tools/interp_runtime_differential.py \
  tools/interp_runtime_target.c tools/interp_routine_reference.c
ci-watch-x86-flight-check: $(wildcard src/*.c src/*.inc include/*.h platform/x86/*) tools/flight_store_build.py \
  tools/flight_table_kinds.py tools/x86_flight_build.py tools/x86_flight_check.py tools/x86_flight_lockstep.c \
  docs/flight-routine-mct.json
