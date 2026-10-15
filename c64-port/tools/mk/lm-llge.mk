# SPDX-License-Identifier: MIT
# Milestone lm-llge (lane F): Luminary099 LUNAR_LANDING_GUIDANCE_EQUATIONS compiled by
# tools/interp_compile.py into native routines of the control-transfer ABI.
LLGE_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c src/lm_normunit.c src/rtb_vector.c src/rtb_unit.c src/guidance_unit_phased.c src/guidance_length_phased.c src/guidance_norm.c src/guidance_product.c tests/interp_case.c
LLGE_SRC = src/lm_llge.c tests/lm_llge_target.c $(LLGE_COMMON)
LLGE_HDR = include/lm_llge.h tests/interp_case.h tests/lm_llge_vectors.h tests/lm_llge_c64_vectors.h
LLGE_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -DIRC_INPUTS -DIRC_CHANNELS -DIRC_LM_HW
.PHONY: test-lm-llge sanitize-lm-llge c64-lm-llge vice-lm-llge lm-llge-differential
all: build/test-lm-llge
test: test-lm-llge
sanitize: sanitize-lm-llge
build/test-lm-llge: $(LLGE_SRC) tests/test_lm_llge.c $(LLGE_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LLGE_SRC) tests/test_lm_llge.c -o $@
test-lm-llge: build/test-lm-llge
	./build/test-lm-llge
sanitize-lm-llge: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(LLGE_SRC) tests/test_lm_llge.c -o build/test-lm-llge-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-llge-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the listing build).
lm-llge-differential:
	python3 tools/interp_routine_differential.py lm-llge --cases 512 --freeze
	git diff --exit-code -- tests/lm_llge_vectors.h tests/lm_llge_c64_vectors.h tools/mk/lm-llge.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
-include tools/mk/lm-llge.groups
LLGE_C64_PRGS = $(foreach g,$(LM_LLGE_GROUPS),build/lm-llge-$(g).prg)
LLGE_C64_SRC = src/lm_llge.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c src/lm_normunit.c src/rtb_vector.c src/rtb_unit.c src/guidance_unit_phased.c src/guidance_length_phased.c src/guidance_norm.c src/guidance_product.c tests/interp_case.c tests/interp_cut.c tests/lm_llge_target.c
c64-lm-llge: $(LLGE_C64_PRGS)
build/lm-llge-%.prg: $(LLGE_SRC) platform/c64/lm_llge_diagnostic.c tools/mk/lm-llge.groups $(LLGE_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(LLGE_MOS_FLAGS) $(LM_LLGE_GROUP_FLAGS_$*) -Wl,-Map,build/lm-llge-$*.map $(LLGE_C64_SRC) platform/c64/lm_llge_diagnostic.c -o $@
vice-lm-llge: $(LLGE_C64_PRGS)
	for p in $(LLGE_C64_PRGS); do echo "PRG $$(basename $$p) sha256 $$(sha256sum $$p | cut -c1-64)"; python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
