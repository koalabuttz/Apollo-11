# SPDX-License-Identifier: MIT
# Milestone lm-ascent (lane D1): Luminary099 ASCENT_GUIDANCE compiled by
# tools/interp_compile.py into native routines of the control-transfer ABI.
ASC_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c
ASC_SRC = src/lm_ascent.c tests/lm_ascent_target.c $(ASC_COMMON)
ASC_HDR = include/lm_ascent.h tests/interp_case.h tests/lm_ascent_vectors.h tests/lm_ascent_c64_vectors.h
ASC_MOS_FLAGS = -std=c99 -Oz -fno-inline -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror
.PHONY: test-lm-ascent sanitize-lm-ascent c64-lm-ascent vice-lm-ascent lm-ascent-differential
all: build/test-lm-ascent
test: test-lm-ascent
sanitize: sanitize-lm-ascent
build/test-lm-ascent: $(ASC_SRC) tests/test_lm_ascent.c $(ASC_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(ASC_SRC) tests/test_lm_ascent.c -o $@
test-lm-ascent: build/test-lm-ascent
	./build/test-lm-ascent
sanitize-lm-ascent: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(ASC_SRC) tests/test_lm_ascent.c -o build/test-lm-ascent-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-ascent-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
lm-ascent-differential:
	python3 tools/interp_routine_differential.py lm-ascent --cases 512 --freeze
	git diff --exit-code -- tests/lm_ascent_vectors.h tests/lm_ascent_c64_vectors.h tools/mk/lm-ascent.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
include tools/mk/lm-ascent.groups
ASC_C64_PRGS = $(foreach g,$(LM_ASCENT_GROUPS),build/lm-ascent-$(g).prg)
ASC_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c tests/lm_ascent_target.c
ASC_C64_lm = src/lm_ascent.c src/lm_interp_orders.c src/lm_interp_steps.c
c64-lm-ascent: $(ASC_C64_PRGS)
build/lm-ascent-%.prg: $(ASC_SRC) platform/c64/lm_ascent_diagnostic.c tools/mk/lm-ascent.groups $(ASC_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(ASC_MOS_FLAGS) $(LM_ASCENT_GROUP_FLAGS_$*) -Wl,-Map,build/lm-ascent-$*.map $(ASC_C64_$(LM_ASCENT_GROUP_MISSION_$*)) $(ASC_C64_COMMON) platform/c64/lm_ascent_diagnostic.c -o $@
vice-lm-ascent: $(ASC_C64_PRGS)
	for p in $(ASC_C64_PRGS); do echo "PRG $$(basename $$p) sha256 $$(sha256sum $$p | cut -c1-64)"; python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
