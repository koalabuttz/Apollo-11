# SPDX-License-Identifier: MIT
# Milestone lm-p70-p71 (lane F): Luminary099 P70-P71 compiled by
# tools/interp_compile.py into native routines of the control-transfer ABI.
P70P71_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c
P70P71_SRC = src/lm_p70_p71.c tests/lm_p70_p71_target.c $(P70P71_COMMON)
P70P71_HDR = include/lm_p70_p71.h tests/interp_case.h tests/lm_p70_p71_vectors.h tests/lm_p70_p71_c64_vectors.h
P70P71_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror
.PHONY: test-lm-p70-p71 sanitize-lm-p70-p71 c64-lm-p70-p71 vice-lm-p70-p71 lm-p70-p71-differential
all: build/test-lm-p70-p71
test: test-lm-p70-p71
sanitize: sanitize-lm-p70-p71
build/test-lm-p70-p71: $(P70P71_SRC) tests/test_lm_p70_p71.c $(P70P71_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(P70P71_SRC) tests/test_lm_p70_p71.c -o $@
test-lm-p70-p71: build/test-lm-p70-p71
	./build/test-lm-p70-p71
sanitize-lm-p70-p71: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(P70P71_SRC) tests/test_lm_p70_p71.c -o build/test-lm-p70-p71-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-p70-p71-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the listing build).
lm-p70-p71-differential:
	python3 tools/interp_routine_differential.py lm-p70-p71 --cases 512 --freeze
	git diff --exit-code -- tests/lm_p70_p71_vectors.h tests/lm_p70_p71_c64_vectors.h tools/mk/lm-p70-p71.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
-include tools/mk/lm-p70-p71.groups
P70P71_C64_PRGS = $(foreach g,$(LM_P70_P71_GROUPS),build/lm-p70-p71-$(g).prg)
P70P71_C64_SRC = src/lm_p70_p71.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c tests/lm_p70_p71_target.c
c64-lm-p70-p71: $(P70P71_C64_PRGS)
build/lm-p70-p71-%.prg: $(P70P71_SRC) platform/c64/lm_p70_p71_diagnostic.c tools/mk/lm-p70-p71.groups $(P70P71_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(P70P71_MOS_FLAGS) $(LM_P70_P71_GROUP_FLAGS_$*) -Wl,-Map,build/lm-p70-p71-$*.map $(P70P71_C64_SRC) platform/c64/lm_p70_p71_diagnostic.c -o $@
vice-lm-p70-p71: $(P70P71_C64_PRGS)
	for p in $(P70P71_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
