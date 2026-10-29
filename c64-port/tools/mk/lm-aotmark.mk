# SPDX-License-Identifier: MIT
# Milestone lm-aotmark (lane D1): Luminary099 AOTMARK compiled by
# tools/interp_compile.py into native routines of the control-transfer ABI.
AOT_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c
AOT_SRC = src/lm_aotmark.c tests/lm_aotmark_target.c $(AOT_COMMON)
AOT_HDR = include/lm_aotmark.h tests/interp_case.h tests/interp_cut.h tests/lm_aotmark_vectors.h tests/lm_aotmark_c64_vectors.h
AOT_MOS_FLAGS = -std=c99 -Oz -fno-inline -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -DIRC_INPUTS
.PHONY: test-lm-aotmark sanitize-lm-aotmark c64-lm-aotmark vice-lm-aotmark lm-aotmark-differential
all: build/test-lm-aotmark
test: test-lm-aotmark
sanitize: sanitize-lm-aotmark
build/test-lm-aotmark: $(AOT_SRC) tests/test_lm_aotmark.c $(AOT_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(AOT_SRC) tests/test_lm_aotmark.c -o $@
test-lm-aotmark: build/test-lm-aotmark
	./build/test-lm-aotmark
sanitize-lm-aotmark: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(AOT_SRC) tests/test_lm_aotmark.c -o build/test-lm-aotmark-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-aotmark-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
lm-aotmark-differential:
	python3 tools/interp_routine_differential.py lm-aotmark --cases 512 --freeze
	git diff --exit-code -- tests/lm_aotmark_vectors.h tests/lm_aotmark_c64_vectors.h tools/mk/lm-aotmark.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
include tools/mk/lm-aotmark.groups
AOT_C64_PRGS = $(foreach g,$(LM_AOTMARK_GROUPS),build/lm-aotmark-$(g).prg)
AOT_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c tests/interp_cut.c tests/lm_aotmark_target.c
AOT_C64_lm = src/lm_aotmark.c src/lm_interp_orders.c src/lm_interp_steps.c
c64-lm-aotmark: $(AOT_C64_PRGS)
build/lm-aotmark-%.prg: $(AOT_SRC) platform/c64/lm_aotmark_diagnostic.c tools/mk/lm-aotmark.groups $(AOT_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(AOT_MOS_FLAGS) $(LM_AOTMARK_GROUP_FLAGS_$*) -Wl,-Map,build/lm-aotmark-$*.map $(AOT_C64_$(LM_AOTMARK_GROUP_MISSION_$*)) $(AOT_C64_COMMON) platform/c64/lm_aotmark_diagnostic.c -o $@
vice-lm-aotmark: $(AOT_C64_PRGS)
	for p in $(AOT_C64_PRGS); do echo "PRG $$(basename $$p) sha256 $$(sha256sum $$p | cut -c1-64)"; python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
