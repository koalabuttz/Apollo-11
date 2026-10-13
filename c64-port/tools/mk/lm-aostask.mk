# SPDX-License-Identifier: MIT
# Milestone lm-aostask (lane D2): Luminary099 AOSTASK_AND_AOSJOB (1/ACCS and
# the AOSTASK/AOSJOB offset filter), compiled by tools/interp_compile.py as
# basic routines at their original addresses ('basic': 'full'; channel
# orders through the program's agc_hardware).
AOS_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
AOS_SRC = src/lm_aostask.c tests/lm_aostask_target.c $(AOS_COMMON)
AOS_HDR = include/lm_aostask.h tests/interp_case.h tests/lm_aostask_vectors.h tests/lm_aostask_c64_vectors.h
# (the spec's c64_flags; -DIRC_CHANNELS for its seeded counters and channels)
AOS_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -fstatic-stack -mlto-zp=110 -DIRC_CHANNELS
.PHONY: test-lm-aostask sanitize-lm-aostask c64-lm-aostask vice-lm-aostask lm-aostask-differential
all: build/test-lm-aostask
test: test-lm-aostask
sanitize: sanitize-lm-aostask
build/test-lm-aostask: $(AOS_SRC) tests/test_lm_aostask.c $(AOS_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(AOS_SRC) tests/test_lm_aostask.c -o $@
test-lm-aostask: build/test-lm-aostask
	./build/test-lm-aostask
sanitize-lm-aostask: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(AOS_SRC) tests/test_lm_aostask.c -o build/test-lm-aostask-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-aostask-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the listing build).
lm-aostask-differential:
	python3 tools/interp_routine_differential.py lm-aostask --cases 256 --freeze
	git diff --exit-code -- tests/lm_aostask_vectors.h tests/lm_aostask_c64_vectors.h tools/mk/lm-aostask.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox, the image under KERNAL).
-include tools/mk/lm-aostask.groups
AOS_C64_PRGS = $(foreach g,$(LM_AOSTASK_GROUPS),build/lm-aostask-$(g).prg)
AOS_C64 = src/lm_aostask.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/lm_aostask_target.c
c64-lm-aostask: $(AOS_C64_PRGS)
build/lm-aostask-%.prg: $(AOS_SRC) platform/c64/lm_aostask_diagnostic.c tools/mk/lm-aostask.groups $(AOS_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(AOS_MOS_FLAGS) $(LM_AOSTASK_GROUP_FLAGS_$*) -Wl,-Map,build/lm-aostask-$*.map $(AOS_C64) platform/c64/lm_aostask_diagnostic.c -o $@
vice-lm-aostask: $(AOS_C64_PRGS)
	for p in $(AOS_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
