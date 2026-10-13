# SPDX-License-Identifier: MIT
# Milestone cm-ephemeris (lane G): Comanche055 LUNAR_AND_SOLAR_EPHEMERIDES_SUBROUTINES compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
CM_EPHEMERIS_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
CM_EPHEMERIS_SRC = src/cm_ephemeris.c tests/cm_ephemeris_target.c $(CM_EPHEMERIS_COMMON)
CM_EPHEMERIS_HDR = include/cm_ephemeris.h tests/interp_case.h tests/cm_ephemeris_vectors.h tests/cm_ephemeris_c64_vectors.h
CM_EPHEMERIS_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -DIRC_CHANNELS
.PHONY: test-cm-ephemeris sanitize-cm-ephemeris c64-cm-ephemeris vice-cm-ephemeris cm-ephemeris-differential
all: build/test-cm-ephemeris
test: test-cm-ephemeris
sanitize: sanitize-cm-ephemeris
build/test-cm-ephemeris: $(CM_EPHEMERIS_SRC) tests/test_cm_ephemeris.c $(CM_EPHEMERIS_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_EPHEMERIS_SRC) tests/test_cm_ephemeris.c -o $@
test-cm-ephemeris: build/test-cm-ephemeris
	./build/test-cm-ephemeris
sanitize-cm-ephemeris: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_EPHEMERIS_SRC) tests/test_cm_ephemeris.c -o build/test-cm-ephemeris-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-ephemeris-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Comanche055 listing build).
cm-ephemeris-differential:
	python3 tools/interp_routine_differential.py cm-ephemeris --cases 256 --freeze
	git diff --exit-code -- tests/cm_ephemeris_vectors.h tests/cm_ephemeris_c64_vectors.h tools/mk/cm-ephemeris.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/cm-ephemeris.groups
CM_EPHEMERIS_C64_PRGS = $(foreach g,$(CM_EPHEMERIS_GROUPS),build/cm-ephemeris-$(g).prg)
CM_EPHEMERIS_C64_SRC = src/cm_ephemeris.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/cm_ephemeris_target.c
c64-cm-ephemeris: $(CM_EPHEMERIS_C64_PRGS)
build/cm-ephemeris-%.prg: $(CM_EPHEMERIS_SRC) platform/c64/cm_ephemeris_diagnostic.c tools/mk/cm-ephemeris.groups $(CM_EPHEMERIS_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CM_EPHEMERIS_MOS_FLAGS) $(CM_EPHEMERIS_GROUP_FLAGS_$*) -Wl,-Map,build/cm-ephemeris-$*.map $(CM_EPHEMERIS_C64_SRC) platform/c64/cm_ephemeris_diagnostic.c -o $@
vice-cm-ephemeris: $(CM_EPHEMERIS_C64_PRGS)
	for p in $(CM_EPHEMERIS_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
