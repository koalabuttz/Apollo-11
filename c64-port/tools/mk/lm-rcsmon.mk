# SPDX-License-Identifier: MIT
# Milestone lm-rcsmon (lane F): Luminary099 RCS_FAILURE_MONITOR compiled by
# tools/interp_compile.py into native routines of the control-transfer ABI.
RCM_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
RCM_SRC = src/lm_rcsmon.c tests/lm_rcsmon_target.c $(RCM_COMMON)
RCM_HDR = include/lm_rcsmon.h tests/interp_case.h tests/lm_rcsmon_vectors.h tests/lm_rcsmon_c64_vectors.h
RCM_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -DIRC_INPUTS
.PHONY: test-lm-rcsmon sanitize-lm-rcsmon c64-lm-rcsmon vice-lm-rcsmon lm-rcsmon-differential
all: build/test-lm-rcsmon
test: test-lm-rcsmon
sanitize: sanitize-lm-rcsmon
build/test-lm-rcsmon: $(RCM_SRC) tests/test_lm_rcsmon.c $(RCM_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(RCM_SRC) tests/test_lm_rcsmon.c -o $@
test-lm-rcsmon: build/test-lm-rcsmon
	./build/test-lm-rcsmon
sanitize-lm-rcsmon: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(RCM_SRC) tests/test_lm_rcsmon.c -o build/test-lm-rcsmon-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-rcsmon-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
lm-rcsmon-differential:
	python3 tools/interp_routine_differential.py lm-rcsmon --cases 512 --freeze
	git diff --exit-code -- tests/lm_rcsmon_vectors.h tests/lm_rcsmon_c64_vectors.h tools/mk/lm-rcsmon.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
-include tools/mk/lm-rcsmon.groups
RCM_C64_PRGS = $(foreach g,$(LM_RCSMON_GROUPS),build/lm-rcsmon-$(g).prg)
RCM_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/lm_rcsmon_target.c
RCM_C64_lm = src/lm_rcsmon.c src/lm_interp_orders.c src/lm_interp_steps.c
c64-lm-rcsmon: $(RCM_C64_PRGS)
build/lm-rcsmon-%.prg: $(RCM_SRC) platform/c64/lm_rcsmon_diagnostic.c tools/mk/lm-rcsmon.groups $(RCM_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(RCM_MOS_FLAGS) $(LM_RCSMON_GROUP_FLAGS_$*) -Wl,-Map,build/lm-rcsmon-$*.map $(RCM_C64_$(LM_RCSMON_GROUP_MISSION_$*)) $(RCM_C64_COMMON) platform/c64/lm_rcsmon_diagnostic.c -o $@
vice-lm-rcsmon: $(RCM_C64_PRGS)
	for p in $(RCM_C64_PRGS); do echo "PRG $$(basename $$p) sha256 $$(sha256sum $$p | cut -c1-64)"; python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
