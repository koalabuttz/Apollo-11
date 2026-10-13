# SPDX-License-Identifier: MIT
# Milestone lm-p40-p47 (lane F): Luminary099 P40-P47 compiled by
# tools/interp_compile.py into native routines of the control-transfer ABI,
# except the channel words, which wait for the output-channel model.
P40_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
P40_SRC = src/lm_p40_p47.c tests/lm_p40_p47_target.c $(P40_COMMON)
P40_HDR = include/lm_p40_p47.h tests/interp_case.h tests/lm_p40_p47_vectors.h tests/lm_p40_p47_c64_vectors.h
P40_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror
.PHONY: test-lm-p40-p47 sanitize-lm-p40-p47 c64-lm-p40-p47 vice-lm-p40-p47 lm-p40-p47-differential
all: build/test-lm-p40-p47
test: test-lm-p40-p47
sanitize: sanitize-lm-p40-p47
build/test-lm-p40-p47: $(P40_SRC) tests/test_lm_p40_p47.c $(P40_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(P40_SRC) tests/test_lm_p40_p47.c -o $@
test-lm-p40-p47: build/test-lm-p40-p47
	./build/test-lm-p40-p47
sanitize-lm-p40-p47: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(P40_SRC) tests/test_lm_p40_p47.c -o build/test-lm-p40-p47-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-p40-p47-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the listing build).
lm-p40-p47-differential:
	python3 tools/interp_routine_differential.py lm-p40-p47 --cases 512 --freeze
	git diff --exit-code -- tests/lm_p40_p47_vectors.h tests/lm_p40_p47_c64_vectors.h tools/mk/lm-p40-p47.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
-include tools/mk/lm-p40-p47.groups
P40_C64_PRGS = $(foreach g,$(LM_P40_P47_GROUPS),build/lm-p40-p47-$(g).prg)
P40_C64_SRC = src/lm_p40_p47.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/lm_p40_p47_target.c
c64-lm-p40-p47: $(P40_C64_PRGS)
build/lm-p40-p47-%.prg: $(P40_SRC) platform/c64/lm_p40_p47_diagnostic.c tools/mk/lm-p40-p47.groups $(P40_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(P40_MOS_FLAGS) $(LM_P40_P47_GROUP_FLAGS_$*) -Wl,-Map,build/lm-p40-p47-$*.map $(P40_C64_SRC) platform/c64/lm_p40_p47_diagnostic.c -o $@
vice-lm-p40-p47: $(P40_C64_PRGS)
	for p in $(P40_C64_PRGS); do echo "PRG $$(basename $$p) sha256 $$(sha256sum $$p | cut -c1-64)"; python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
