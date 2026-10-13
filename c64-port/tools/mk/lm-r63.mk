# SPDX-License-Identifier: MIT
# Milestone lm-r63 (lane F): Luminary099 R63 compiled by
# tools/interp_compile.py into native routines of the control-transfer ABI.
R63_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c
R63_SRC = src/lm_r63.c tests/lm_r63_target.c $(R63_COMMON)
R63_HDR = include/lm_r63.h tests/interp_case.h tests/lm_r63_vectors.h tests/lm_r63_c64_vectors.h
R63_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror
.PHONY: test-lm-r63 sanitize-lm-r63 c64-lm-r63 vice-lm-r63 lm-r63-differential
all: build/test-lm-r63
test: test-lm-r63
sanitize: sanitize-lm-r63
build/test-lm-r63: $(R63_SRC) tests/test_lm_r63.c $(R63_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(R63_SRC) tests/test_lm_r63.c -o $@
test-lm-r63: build/test-lm-r63
	./build/test-lm-r63
sanitize-lm-r63: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(R63_SRC) tests/test_lm_r63.c -o build/test-lm-r63-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-r63-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the listing build).
lm-r63-differential:
	python3 tools/interp_routine_differential.py lm-r63 --cases 512 --freeze
	git diff --exit-code -- tests/lm_r63_vectors.h tests/lm_r63_c64_vectors.h tools/mk/lm-r63.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
-include tools/mk/lm-r63.groups
R63_C64_PRGS = $(foreach g,$(LM_R63_GROUPS),build/lm-r63-$(g).prg)
R63_C64_SRC = src/lm_r63.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c tests/lm_r63_target.c
c64-lm-r63: $(R63_C64_PRGS)
build/lm-r63-%.prg: $(R63_SRC) platform/c64/lm_r63_diagnostic.c tools/mk/lm-r63.groups $(R63_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(R63_MOS_FLAGS) $(LM_R63_GROUP_FLAGS_$*) -Wl,-Map,build/lm-r63-$*.map $(R63_C64_SRC) platform/c64/lm_r63_diagnostic.c -o $@
vice-lm-r63: $(R63_C64_PRGS)
	for p in $(R63_C64_PRGS); do echo "PRG $$(basename $$p) sha256 $$(sha256sum $$p | cut -c1-64)"; python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
