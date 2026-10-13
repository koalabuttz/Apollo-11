# SPDX-License-Identifier: MIT
# Runtime interpreter (Interpreter lane; INTERPRETER.agc over the rope image,
# src/lm_interp_runtime.c and src/cm_interp_runtime.c).
# interp-runtime-check: include/interp_runtime_tables.h matches the listings.
# interp-runtime-differential: every compiled-routine milestone's cases
# (tools/interp_specs.py) with its interpretive points run by the interpreter,
# against yaAGC (INTERP_RUNTIME_CASES cases per routine, default 8).
# interp-runtime-size: each mission's interpreter linked as one ABI 2
# job-slot module with the flight store's flags (build/interp-runtime/size.json).
# The differential and size need the listings, ropes and yaAGC objects, as the
# compiled-routine differentials do; neither gates `test` yet.
INTERP_RUNTIME_CASES ?= 8
.PHONY: interp-runtime-check interp-runtime-differential interp-runtime-size
interp-runtime-check:
	python3 tools/interp_runtime_generate.py --check
interp-runtime-differential: interp-runtime-check build/agc_engine_hardware_dv.o | build
	python3 tools/interp_runtime_differential.py --cases $(INTERP_RUNTIME_CASES)
interp-runtime-size: interp-runtime-check | build
	python3 tools/interp_runtime_size.py
