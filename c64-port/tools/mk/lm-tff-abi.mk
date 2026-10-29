# SPDX-License-Identifier: MIT
# Milestone lm-tff-abi (lane E): Luminary099 TIME_OF_FREE_FALL compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
LM_TFF_ABI_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/lm_integration_moves.c tests/interp_case.c
LM_TFF_ABI_SRC = src/lm_tff_abi.c tests/lm_tff_abi_target.c $(LM_TFF_ABI_COMMON)
LM_TFF_ABI_HDR = include/lm_tff_abi.h tests/interp_case.h tests/lm_tff_abi_vectors.h tests/lm_tff_abi_c64_vectors.h
LM_TFF_ABI_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -DIRC_CHANNELS -DAGC_TRANSFER_DANZIG_MARKER
.PHONY: test-lm-tff-abi sanitize-lm-tff-abi c64-lm-tff-abi vice-lm-tff-abi lm-tff-abi-differential
all: build/test-lm-tff-abi
test: test-lm-tff-abi
sanitize: sanitize-lm-tff-abi
build/test-lm-tff-abi: $(LM_TFF_ABI_SRC) tests/test_lm_tff_abi.c $(LM_TFF_ABI_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_TFF_ABI_SRC) tests/test_lm_tff_abi.c -o $@
test-lm-tff-abi: build/test-lm-tff-abi
	./build/test-lm-tff-abi
sanitize-lm-tff-abi: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(LM_TFF_ABI_SRC) tests/test_lm_tff_abi.c -o build/test-lm-tff-abi-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-tff-abi-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Comanche055 listing build).
lm-tff-abi-differential:
	python3 tools/interp_routine_differential.py lm-tff-abi --cases 256 --freeze
	git diff --exit-code -- tests/lm_tff_abi_vectors.h tests/lm_tff_abi_c64_vectors.h tools/mk/lm-tff-abi.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/lm-tff-abi.groups
LM_TFF_ABI_C64_PRGS = $(foreach g,$(LM_TFF_ABI_GROUPS),build/lm-tff-abi-$(g).prg)
LM_TFF_ABI_C64_SRC = src/lm_tff_abi.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/lm_integration_moves.c tests/interp_case.c tests/lm_tff_abi_target.c
c64-lm-tff-abi: $(LM_TFF_ABI_C64_PRGS)
build/lm-tff-abi-%.prg: $(LM_TFF_ABI_SRC) platform/c64/lm_tff_abi_diagnostic.c tools/mk/lm-tff-abi.groups $(LM_TFF_ABI_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(LM_TFF_ABI_MOS_FLAGS) $(LM_TFF_ABI_GROUP_FLAGS_$*) -Wl,-Map,build/lm-tff-abi-$*.map $(LM_TFF_ABI_C64_SRC) platform/c64/lm_tff_abi_diagnostic.c -o $@
vice-lm-tff-abi: $(LM_TFF_ABI_C64_PRGS)
	for p in $(LM_TFF_ABI_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
