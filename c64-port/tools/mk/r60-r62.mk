# SPDX-License-Identifier: MIT
# Milestone r60-r62 (lane D1b): Luminary099 and Comanche055
# R60,R62 (both missions) compiled by tools/interp_compile.py into
# native routines of the control-transfer ABI.
R60_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c
R60_SRC = src/lm_r60_r62.c src/cm_r60_r62.c tests/r60_r62_target.c $(R60_COMMON)
R60_HDR = include/lm_r60_r62.h include/cm_r60_r62.h tests/interp_case.h tests/r60_r62_vectors.h tests/r60_r62_c64_vectors.h
R60_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror
.PHONY: test-r60-r62 sanitize-r60-r62 c64-r60-r62 vice-r60-r62 r60-r62-differential
all: build/test-r60-r62
test: test-r60-r62
sanitize: sanitize-r60-r62
build/test-r60-r62: $(R60_SRC) tests/test_r60_r62.c $(R60_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(R60_SRC) tests/test_r60_r62.c -o $@
test-r60-r62: build/test-r60-r62
	./build/test-r60-r62
sanitize-r60-r62: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(R60_SRC) tests/test_r60_r62.c -o build/test-r60-r62-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-r60-r62-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
r60-r62-differential:
	python3 tools/interp_routine_differential.py r60-r62 --cases 512 --freeze
	git diff --exit-code -- tests/r60_r62_vectors.h tests/r60_r62_c64_vectors.h tools/mk/r60-r62.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
include tools/mk/r60-r62.groups
R60_C64_PRGS = $(foreach g,$(R60_R62_GROUPS),build/r60-r62-$(g).prg)
R60_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c tests/r60_r62_target.c
R60_C64_lm = src/lm_r60_r62.c src/lm_interp_orders.c src/lm_interp_steps.c
R60_C64_cm = src/cm_r60_r62.c src/cm_interp_orders.c src/cm_interp_steps.c
c64-r60-r62: $(R60_C64_PRGS)
build/r60-r62-%.prg: $(R60_SRC) platform/c64/r60_r62_diagnostic.c tools/mk/r60-r62.groups $(R60_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(R60_MOS_FLAGS) $(R60_R62_GROUP_FLAGS_$*) -Wl,-Map,build/r60-r62-$*.map $(R60_C64_$(R60_R62_GROUP_MISSION_$*)) $(R60_C64_COMMON) platform/c64/r60_r62_diagnostic.c -o $@
vice-r60-r62: $(R60_C64_PRGS)
	for p in $(R60_C64_PRGS); do echo "PRG $$(basename $$p) sha256 $$(sha256sum $$p | cut -c1-64)"; python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
