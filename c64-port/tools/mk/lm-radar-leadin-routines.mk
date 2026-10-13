# SPDX-License-Identifier: MIT
# Milestone lm-radar-leadin-routines (lane D2): Luminary099
# RADAR_LEADIN_ROUTINES (RADSAMP, DORSAMP and VARADAR, the radar sampling
# lead-ins), compiled by tools/interp_compile.py as basic routines at their
# original addresses ('basic': 'full'; channel orders through the program's
# agc_hardware).
RLR_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
RLR_SRC = src/lm_radar_leadin_routines.c tests/lm_radar_leadin_routines_target.c $(RLR_COMMON)
RLR_HDR = include/lm_radar_leadin_routines.h tests/interp_case.h tests/lm_radar_leadin_routines_vectors.h tests/lm_radar_leadin_routines_c64_vectors.h
# (the spec's c64_flags; -DIRC_CHANNELS for its seeded counters and channels,
# -DIRC_INPUTS for the latched input channels)
RLR_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -fstatic-stack -mlto-zp=110 -DIRC_CHANNELS -DIRC_INPUTS
.PHONY: test-lm-radar-leadin-routines sanitize-lm-radar-leadin-routines c64-lm-radar-leadin-routines vice-lm-radar-leadin-routines lm-radar-leadin-routines-differential
all: build/test-lm-radar-leadin-routines
test: test-lm-radar-leadin-routines
sanitize: sanitize-lm-radar-leadin-routines
build/test-lm-radar-leadin-routines: $(RLR_SRC) tests/test_lm_radar_leadin_routines.c $(RLR_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(RLR_SRC) tests/test_lm_radar_leadin_routines.c -o $@
test-lm-radar-leadin-routines: build/test-lm-radar-leadin-routines
	./build/test-lm-radar-leadin-routines
sanitize-lm-radar-leadin-routines: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(RLR_SRC) tests/test_lm_radar_leadin_routines.c -o build/test-lm-radar-leadin-routines-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-radar-leadin-routines-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the listing build).
lm-radar-leadin-routines-differential:
	python3 tools/interp_routine_differential.py lm-radar-leadin-routines --cases 256 --freeze
	git diff --exit-code -- tests/lm_radar_leadin_routines_vectors.h tests/lm_radar_leadin_routines_c64_vectors.h tools/mk/lm-radar-leadin-routines.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox, the image under KERNAL).
-include tools/mk/lm-radar-leadin-routines.groups
RLR_C64_PRGS = $(foreach g,$(LM_RADAR_LEADIN_ROUTINES_GROUPS),build/lm-radar-leadin-routines-$(g).prg)
RLR_C64 = src/lm_radar_leadin_routines.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/lm_radar_leadin_routines_target.c
c64-lm-radar-leadin-routines: $(RLR_C64_PRGS)
build/lm-radar-leadin-routines-%.prg: $(RLR_SRC) platform/c64/lm_radar_leadin_routines_diagnostic.c tools/mk/lm-radar-leadin-routines.groups $(RLR_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(RLR_MOS_FLAGS) $(LM_RADAR_LEADIN_ROUTINES_GROUP_FLAGS_$*) -Wl,-Map,build/lm-radar-leadin-routines-$*.map $(RLR_C64) platform/c64/lm_radar_leadin_routines_diagnostic.c -o $@
vice-lm-radar-leadin-routines: $(RLR_C64_PRGS)
	for p in $(RLR_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
