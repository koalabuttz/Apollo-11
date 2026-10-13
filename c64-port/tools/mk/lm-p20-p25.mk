# SPDX-License-Identifier: MIT
# Milestone lm-p20-p25 (lane B): Luminary099 P20-P25 (rendezvous navigation, R21-R25 radar, V67) compiled by
# tools/interp_compile.py into native routines of the control-transfer ABI.
LP20_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c
LP20_SRC = src/lm_p20_p25.c tests/lm_p20_p25_target.c $(LP20_COMMON)
LP20_HDR = include/lm_p20_p25.h tests/interp_case.h tests/lm_p20_p25_vectors.h tests/lm_p20_p25_c64_vectors.h
LP20_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -fstatic-stack -mlto-zp=110 -DIRC_INPUTS
.PHONY: test-lm-p20-p25 sanitize-lm-p20-p25 c64-lm-p20-p25 vice-lm-p20-p25 lm-p20-p25-differential
all: build/test-lm-p20-p25
test: test-lm-p20-p25
sanitize: sanitize-lm-p20-p25
build/test-lm-p20-p25: $(LP20_SRC) tests/test_lm_p20_p25.c $(LP20_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LP20_SRC) tests/test_lm_p20_p25.c -o $@
test-lm-p20-p25: build/test-lm-p20-p25
	./build/test-lm-p20-p25
sanitize-lm-p20-p25: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(LP20_SRC) tests/test_lm_p20_p25.c -o build/test-lm-p20-p25-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-p20-p25-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
lm-p20-p25-differential:
	python3 tools/interp_routine_differential.py lm-p20-p25 --cases 128 --freeze
	git diff --exit-code -- tests/lm_p20_p25_vectors.h tests/lm_p20_p25_c64_vectors.h tools/mk/lm-p20-p25.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
include tools/mk/lm-p20-p25.groups
LP20_C64_PRGS = $(foreach g,$(LM_P20_P25_GROUPS),build/lm-p20-p25-$(g).prg)
LP20_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c tests/lm_p20_p25_target.c
LP20_C64_lm = src/lm_p20_p25.c src/lm_interp_orders.c src/lm_interp_steps.c
c64-lm-p20-p25: $(LP20_C64_PRGS)
build/lm-p20-p25-%.prg: $(LP20_SRC) platform/c64/lm_p20_p25_diagnostic.c tools/mk/lm-p20-p25.groups $(LP20_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(LP20_MOS_FLAGS) $(LM_P20_P25_GROUP_FLAGS_$*) -Wl,-Map,build/lm-p20-p25-$*.map $(LP20_C64_$(LM_P20_P25_GROUP_MISSION_$*)) $(LP20_C64_COMMON) platform/c64/lm_p20_p25_diagnostic.c -o $@
vice-lm-p20-p25: $(LP20_C64_PRGS)
	for p in $(LP20_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
