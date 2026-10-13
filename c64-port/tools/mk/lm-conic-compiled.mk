# SPDX-License-Identifier: MIT
# Milestone lm-conic-compiled (lane B): Luminary099 CONIC_SUBROUTINES compiled by
# tools/interp_compile.py into native routines of the control-transfer ABI.
LCC_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/lm_conic_checkctr.c tests/interp_case.c
LCC_SRC = src/lm_conic_compiled.c tests/lm_conic_compiled_target.c $(LCC_COMMON)
LCC_HDR = include/lm_conic_compiled.h tests/interp_case.h tests/lm_conic_compiled_vectors.h tests/lm_conic_compiled_c64_vectors.h
LCC_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -fstatic-stack -mlto-zp=110
.PHONY: test-lm-conic-compiled sanitize-lm-conic-compiled c64-lm-conic-compiled vice-lm-conic-compiled lm-conic-compiled-differential
all: build/test-lm-conic-compiled
test: test-lm-conic-compiled
sanitize: sanitize-lm-conic-compiled
build/test-lm-conic-compiled: $(LCC_SRC) tests/test_lm_conic_compiled.c $(LCC_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LCC_SRC) tests/test_lm_conic_compiled.c -o $@
test-lm-conic-compiled: build/test-lm-conic-compiled
	./build/test-lm-conic-compiled
sanitize-lm-conic-compiled: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(LCC_SRC) tests/test_lm_conic_compiled.c -o build/test-lm-conic-compiled-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-conic-compiled-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the listing build).
lm-conic-compiled-differential:
	python3 tools/interp_routine_differential.py lm-conic-compiled --cases 64 --freeze
	git diff --exit-code -- tests/lm_conic_compiled_vectors.h tests/lm_conic_compiled_c64_vectors.h tools/mk/lm-conic-compiled.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
include tools/mk/lm-conic-compiled.groups
LCC_C64_PRGS = $(foreach g,$(LM_CONIC_COMPILED_GROUPS),build/lm-conic-compiled-$(g).prg)
LCC_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/lm_conic_checkctr.c tests/interp_case.c tests/lm_conic_compiled_target.c
LCC_C64_lm = src/lm_conic_compiled.c src/lm_interp_orders.c src/lm_interp_steps.c
c64-lm-conic-compiled: $(LCC_C64_PRGS)
build/lm-conic-compiled-%.prg: $(LCC_SRC) platform/c64/lm_conic_compiled_diagnostic.c tools/mk/lm-conic-compiled.groups $(LCC_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(LCC_MOS_FLAGS) $(LM_CONIC_COMPILED_GROUP_FLAGS_$*) -Wl,-Map,build/lm-conic-compiled-$*.map $(LCC_C64_$(LM_CONIC_COMPILED_GROUP_MISSION_$*)) $(LCC_C64_COMMON) platform/c64/lm_conic_compiled_diagnostic.c -o $@
vice-lm-conic-compiled: $(LCC_C64_PRGS)
	for p in $(LCC_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
