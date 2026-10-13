# SPDX-License-Identifier: MIT
# Milestone r30 (lane E): Luminary099 and Comanche055 R30 compiled by
# tools/interp_compile.py into native routines of the control-transfer ABI.
R30_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c
R30_SRC = src/lm_r30.c src/cm_r30.c tests/r30_target.c $(R30_COMMON)
R30_HDR = include/lm_r30.h include/cm_r30.h tests/interp_case.h tests/r30_vectors.h tests/r30_c64_vectors.h
R30_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror
.PHONY: test-r30 sanitize-r30 c64-r30 vice-r30 r30-differential
all: build/test-r30
test: test-r30
sanitize: sanitize-r30
build/test-r30: $(R30_SRC) tests/test_r30.c $(R30_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(R30_SRC) tests/test_r30.c -o $@
test-r30: build/test-r30
	./build/test-r30
sanitize-r30: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(R30_SRC) tests/test_r30.c -o build/test-r30-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-r30-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
r30-differential:
	python3 tools/interp_routine_differential.py r30 --cases 512 --freeze
	git diff --exit-code -- tests/r30_vectors.h tests/r30_c64_vectors.h tools/mk/r30.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
include tools/mk/r30.groups
R30_C64_PRGS = $(foreach g,$(R30_GROUPS),build/r30-$(g).prg)
R30_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c tests/r30_target.c
R30_C64_lm = src/lm_r30.c src/lm_interp_orders.c src/lm_interp_steps.c
R30_C64_cm = src/cm_r30.c src/cm_interp_orders.c src/cm_interp_steps.c
c64-r30: $(R30_C64_PRGS)
build/r30-%.prg: $(R30_SRC) platform/c64/r30_diagnostic.c tools/mk/r30.groups $(R30_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(R30_MOS_FLAGS) $(R30_GROUP_FLAGS_$*) -Wl,-Map,build/r30-$*.map $(R30_C64_$(R30_GROUP_MISSION_$*)) $(R30_C64_COMMON) platform/c64/r30_diagnostic.c -o $@
vice-r30: $(R30_C64_PRGS)
	for p in $(R30_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
