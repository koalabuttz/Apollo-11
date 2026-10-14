# SPDX-License-Identifier: MIT
# Runtime interpreter (Interpreter lane; INTERPRETER.agc over the rope image).
# Verification interpreter: src/lm_interp_runtime.c and src/cm_interp_runtime.c
# (the image-level lmi_ bodies). Compact interpreter, the flight build:
# src/lm_interp_core.c and src/cm_interp_core.c.
# interp-runtime-check: include/interp_runtime_tables.h matches the listings.
# interp-runtime-differential: every compiled-routine milestone's cases
# (tools/interp_specs.py) with its interpretive points run by the interpreter,
# against yaAGC (INTERP_RUNTIME_CASES cases per routine, default 8);
# interp-core-differential the same with the compact interpreter.
# interp-runtime-size: each mission's compact interpreter linked as one ABI 2
# job-slot module with the flight store's flags (build/interp-runtime/size.json).
# interp-core-fuzz: random interpretive programs, the compact interpreter
# against the verification one (INTERP_CORE_FUZZ cases, default 100000),
# built both ways: with its own image ops, and with the flight module's
# INTERP_K_OPS (the resident's k_ ops; host stand-ins tools/interp_core_kops.c).
# interp-core-cycles: 6502 cycles per order (build/interp-cycles/<m>.json).
# vice-interp-core: the flight build (k_ ops, the $D000 extension) on a PAL
# C64 in VICE, fuzz slot records checked against the host's, IRQs running.
# The differentials and size need the listings, ropes and yaAGC objects, as
# the compiled-routine differentials do; none gates `test` yet.
INTERP_RUNTIME_CASES ?= 8
INTERP_CORE_FUZZ ?= 100000
INTERP_CORE_FUZZ_SRC := tools/interp_core_fuzz.c src/lm_interp_runtime.c src/cm_interp_runtime.c \
  src/lm_interp_core.c src/cm_interp_core.c src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c \
  src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c
.PHONY: interp-runtime-check interp-runtime-differential interp-core-differential interp-runtime-size \
  interp-core-fuzz interp-core-cycles vice-interp-core interp-flight-vice interp-flight-flowed-vice
interp-runtime-check:
	python3 tools/interp_runtime_generate.py --check
interp-runtime-differential: interp-runtime-check build/agc_engine_hardware_dv.o | build
	python3 tools/interp_runtime_differential.py --cases $(INTERP_RUNTIME_CASES)
interp-core-differential: interp-runtime-check build/agc_engine_hardware_dv.o | build
	python3 tools/interp_runtime_differential.py --core --cases $(INTERP_RUNTIME_CASES)
interp-runtime-size: interp-runtime-check | build
	python3 tools/interp_runtime_size.py
build/interp-core-fuzz: $(INTERP_CORE_FUZZ_SRC) | build
	$(CC) -std=gnu99 -O2 -Wall -Wextra -Werror -Iinclude -Itests -Dexit=interp_core_fuzz_exit $(INTERP_CORE_FUZZ_SRC) -o $@
build/interp-core-fuzz-kops: $(INTERP_CORE_FUZZ_SRC) tools/interp_core_kops.c | build
	$(CC) -std=gnu99 -O2 -Wall -Wextra -Werror -Iinclude -Itests -Dexit=interp_core_fuzz_exit -DINTERP_K_OPS=1 -DINTERP_K_OPS2=1 \
	  $(INTERP_CORE_FUZZ_SRC) tools/interp_core_kops.c -o $@
interp-core-fuzz: build/interp-core-fuzz build/interp-core-fuzz-kops build/fixed-data/Luminary099/MAIN.agc.bin \
  build/fixed-data/Comanche055/MAIN.agc.bin
	build/interp-core-fuzz build/fixed-data/Luminary099/MAIN.agc.bin build/fixed-data/Comanche055/MAIN.agc.bin $(INTERP_CORE_FUZZ) 11
	build/interp-core-fuzz-kops build/fixed-data/Luminary099/MAIN.agc.bin build/fixed-data/Comanche055/MAIN.agc.bin $(INTERP_CORE_FUZZ) 12
interp-core-cycles: build/interp-core-fuzz build/fixed-data/Luminary099/MAIN.agc.bin build/fixed-data/Comanche055/MAIN.agc.bin
	python3 tools/interp_core_cycles.py
vice-interp-core: build/interp-core-fuzz build/fixed-data/Luminary099/MAIN.agc.bin build/fixed-data/Comanche055/MAIN.agc.bin
	python3 tools/interp_core_vice_test.py lm
	python3 tools/interp_core_vice_test.py cm
# interp-flight-vice: flight-scenario interpretive code (yaAGC's LM and CM
# P00 from Fresh Start, at each arrival at an interpreter entry point)
# through the flight resident and the store's interpreter in VICE, each
# result a state yaAGC reaches (tools/interp_flight_test.py). Like
# flight-store-vice it needs the whole store and does not gate `test`.
interp-flight-vice: flight-store
	python3 tools/interp_flight_test.py
# interp-flight-flowed-vice: flowed states, not mission flight: each
# FLIGHT_INTERP_STOP module's milestone cases run in yaAGC to every first
# arrival at an interpreter entry point inside the file
# (tools/interp_flight_flowed.c), those the verification interpreter keeps
# to the contract for (tools/interp_flight_verify.c), through the flight
# store in VICE as interp-flight-vice runs them, several boots.
interp-flight-flowed-vice: flight-store build/agc_engine_hardware_dv.o
	python3 tools/interp_flight_test.py --missions lm --flowed
