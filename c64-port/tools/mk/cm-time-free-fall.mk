# SPDX-License-Identifier: MIT
# Milestone cm-time-free-fall (lane G): Comanche055 CM_TIME_FREE_FALL compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
CM_TIME_FREE_FALL_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/cm_integration_moves.c tests/interp_case.c
CM_TIME_FREE_FALL_SRC = src/cm_time_free_fall.c tests/cm_time_free_fall_target.c $(CM_TIME_FREE_FALL_COMMON)
CM_TIME_FREE_FALL_HDR = include/cm_time_free_fall.h tests/interp_case.h tests/cm_time_free_fall_vectors.h tests/cm_time_free_fall_c64_vectors.h
CM_TIME_FREE_FALL_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -DIRC_CHANNELS -DAGC_CHANNELS_HW=irc_cm_hw -DAGC_TRANSFER_DANZIG_MARKER
.PHONY: test-cm-time-free-fall sanitize-cm-time-free-fall c64-cm-time-free-fall vice-cm-time-free-fall cm-time-free-fall-differential
all: build/test-cm-time-free-fall
test: test-cm-time-free-fall
sanitize: sanitize-cm-time-free-fall
build/test-cm-time-free-fall: $(CM_TIME_FREE_FALL_SRC) tests/test_cm_time_free_fall.c $(CM_TIME_FREE_FALL_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_TIME_FREE_FALL_SRC) tests/test_cm_time_free_fall.c -o $@
test-cm-time-free-fall: build/test-cm-time-free-fall
	./build/test-cm-time-free-fall
sanitize-cm-time-free-fall: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_TIME_FREE_FALL_SRC) tests/test_cm_time_free_fall.c -o build/test-cm-time-free-fall-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-time-free-fall-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Comanche055 listing build).
cm-time-free-fall-differential:
	python3 tools/interp_routine_differential.py cm-time-free-fall --cases 256 --freeze
	git diff --exit-code -- tests/cm_time_free_fall_vectors.h tests/cm_time_free_fall_c64_vectors.h tools/mk/cm-time-free-fall.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/cm-time-free-fall.groups
CM_TIME_FREE_FALL_C64_PRGS = $(foreach g,$(CM_TIME_FREE_FALL_GROUPS),build/cm-time-free-fall-$(g).prg)
CM_TIME_FREE_FALL_C64_SRC = src/cm_time_free_fall.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/cm_integration_moves.c tests/interp_case.c tests/cm_time_free_fall_target.c
c64-cm-time-free-fall: $(CM_TIME_FREE_FALL_C64_PRGS)
build/cm-time-free-fall-%.prg: $(CM_TIME_FREE_FALL_SRC) platform/c64/cm_time_free_fall_diagnostic.c tools/mk/cm-time-free-fall.groups $(CM_TIME_FREE_FALL_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CM_TIME_FREE_FALL_MOS_FLAGS) $(CM_TIME_FREE_FALL_GROUP_FLAGS_$*) -Wl,-Map,build/cm-time-free-fall-$*.map $(CM_TIME_FREE_FALL_C64_SRC) platform/c64/cm_time_free_fall_diagnostic.c -o $@
vice-cm-time-free-fall: $(CM_TIME_FREE_FALL_C64_PRGS)
	for p in $(CM_TIME_FREE_FALL_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
