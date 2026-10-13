# SPDX-License-Identifier: MIT
# Milestone lm-kalman (lane D2): Luminary099 KALMAN_FILTER (RATELOOP, the
# DAP jet torque and rate filter feed), compiled by tools/interp_compile.py
# as basic routines at their original addresses ('basic': 'full'; channel
# orders through the program's agc_hardware).
KAL_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
KAL_SRC = src/lm_kalman.c tests/lm_kalman_target.c $(KAL_COMMON)
KAL_HDR = include/lm_kalman.h tests/interp_case.h tests/lm_kalman_vectors.h tests/lm_kalman_c64_vectors.h
# (the spec's c64_flags; -DIRC_CHANNELS for its seeded counters and channels,
# -DIRC_INPUTS for the latched input channels)
KAL_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -fstatic-stack -mlto-zp=110 -DIRC_CHANNELS -DIRC_INPUTS
.PHONY: test-lm-kalman sanitize-lm-kalman c64-lm-kalman vice-lm-kalman lm-kalman-differential
all: build/test-lm-kalman
test: test-lm-kalman
sanitize: sanitize-lm-kalman
build/test-lm-kalman: $(KAL_SRC) tests/test_lm_kalman.c $(KAL_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(KAL_SRC) tests/test_lm_kalman.c -o $@
test-lm-kalman: build/test-lm-kalman
	./build/test-lm-kalman
sanitize-lm-kalman: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(KAL_SRC) tests/test_lm_kalman.c -o build/test-lm-kalman-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-kalman-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the listing build).
lm-kalman-differential:
	python3 tools/interp_routine_differential.py lm-kalman --cases 256 --freeze
	git diff --exit-code -- tests/lm_kalman_vectors.h tests/lm_kalman_c64_vectors.h tools/mk/lm-kalman.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox, the image under KERNAL).
-include tools/mk/lm-kalman.groups
KAL_C64_PRGS = $(foreach g,$(LM_KALMAN_GROUPS),build/lm-kalman-$(g).prg)
KAL_C64 = src/lm_kalman.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/lm_kalman_target.c
c64-lm-kalman: $(KAL_C64_PRGS)
build/lm-kalman-%.prg: $(KAL_SRC) platform/c64/lm_kalman_diagnostic.c tools/mk/lm-kalman.groups $(KAL_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(KAL_MOS_FLAGS) $(LM_KALMAN_GROUP_FLAGS_$*) -Wl,-Map,build/lm-kalman-$*.map $(KAL_C64) platform/c64/lm_kalman_diagnostic.c -o $@
vice-lm-kalman: $(KAL_C64_PRGS)
	for p in $(KAL_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
