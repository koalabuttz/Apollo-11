# SPDX-License-Identifier: MIT
# Milestone cm-reentry (lane G): Comanche055 REENTRY_CONTROL compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
CM_REENTRY_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c
CM_REENTRY_SRC = src/cm_reentry.c tests/cm_reentry_target.c $(CM_REENTRY_COMMON)
CM_REENTRY_HDR = include/cm_reentry.h tests/interp_case.h tests/cm_reentry_vectors.h tests/cm_reentry_c64_vectors.h
CM_REENTRY_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -DIRC_CHANNELS
.PHONY: test-cm-reentry sanitize-cm-reentry c64-cm-reentry vice-cm-reentry cm-reentry-differential
all: build/test-cm-reentry
test: test-cm-reentry
sanitize: sanitize-cm-reentry
build/test-cm-reentry: $(CM_REENTRY_SRC) tests/test_cm_reentry.c $(CM_REENTRY_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_REENTRY_SRC) tests/test_cm_reentry.c -o $@
test-cm-reentry: build/test-cm-reentry
	./build/test-cm-reentry
sanitize-cm-reentry: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_REENTRY_SRC) tests/test_cm_reentry.c -o build/test-cm-reentry-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-reentry-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Comanche055 listing build).
cm-reentry-differential:
	python3 tools/interp_routine_differential.py cm-reentry --cases 256 --freeze
	git diff --exit-code -- tests/cm_reentry_vectors.h tests/cm_reentry_c64_vectors.h tools/mk/cm-reentry.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/cm-reentry.groups
CM_REENTRY_C64_PRGS = $(foreach g,$(CM_REENTRY_GROUPS),build/cm-reentry-$(g).prg)
CM_REENTRY_C64_SRC = src/cm_reentry.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c tests/cm_reentry_target.c
c64-cm-reentry: $(CM_REENTRY_C64_PRGS)
build/cm-reentry-%.prg: $(CM_REENTRY_SRC) platform/c64/cm_reentry_diagnostic.c tools/mk/cm-reentry.groups $(CM_REENTRY_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CM_REENTRY_MOS_FLAGS) $(CM_REENTRY_GROUP_FLAGS_$*) -Wl,-Map,build/cm-reentry-$*.map $(CM_REENTRY_C64_SRC) platform/c64/cm_reentry_diagnostic.c -o $@
vice-cm-reentry: $(CM_REENTRY_C64_PRGS)
	for p in $(CM_REENTRY_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
