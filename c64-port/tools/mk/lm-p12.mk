# SPDX-License-Identifier: MIT
# Milestone lm-p12 (lane F): Luminary099 P12 compiled by
# tools/interp_compile.py into native routines of the control-transfer ABI.
P12_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c
P12_SRC = src/lm_p12.c tests/lm_p12_target.c $(P12_COMMON)
P12_HDR = include/lm_p12.h tests/interp_case.h tests/lm_p12_vectors.h tests/lm_p12_c64_vectors.h
P12_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror
.PHONY: test-lm-p12 sanitize-lm-p12 c64-lm-p12 vice-lm-p12 lm-p12-differential
all: build/test-lm-p12
test: test-lm-p12
sanitize: sanitize-lm-p12
build/test-lm-p12: $(P12_SRC) tests/test_lm_p12.c $(P12_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(P12_SRC) tests/test_lm_p12.c -o $@
test-lm-p12: build/test-lm-p12
	./build/test-lm-p12
sanitize-lm-p12: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(P12_SRC) tests/test_lm_p12.c -o build/test-lm-p12-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-p12-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the listing build).
lm-p12-differential:
	python3 tools/interp_routine_differential.py lm-p12 --cases 512 --freeze
	git diff --exit-code -- tests/lm_p12_vectors.h tests/lm_p12_c64_vectors.h tools/mk/lm-p12.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
-include tools/mk/lm-p12.groups
P12_C64_PRGS = $(foreach g,$(LM_P12_GROUPS),build/lm-p12-$(g).prg)
P12_C64_SRC = src/lm_p12.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c tests/lm_p12_target.c
c64-lm-p12: $(P12_C64_PRGS)
build/lm-p12-%.prg: $(P12_SRC) platform/c64/lm_p12_diagnostic.c tools/mk/lm-p12.groups $(P12_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(P12_MOS_FLAGS) $(LM_P12_GROUP_FLAGS_$*) -Wl,-Map,build/lm-p12-$*.map $(P12_C64_SRC) platform/c64/lm_p12_diagnostic.c -o $@
vice-lm-p12: $(P12_C64_PRGS)
	for p in $(P12_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
