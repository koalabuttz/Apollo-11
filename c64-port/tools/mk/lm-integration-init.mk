# SPDX-License-Identifier: MIT
# Milestone lm-integration-init (lane B): Luminary099 INTEGRATION_INITIALIZATION compiled by
# tools/interp_compile.py into native routines of the control-transfer ABI.
LII_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c src/rtb_agreement.c src/lm_integration_moves.c tests/interp_case.c
LII_SRC = src/lm_integration_init.c tests/lm_integration_init_target.c $(LII_COMMON)
LII_HDR = include/lm_integration_init.h tests/interp_case.h tests/lm_integration_init_vectors.h tests/lm_integration_init_c64_vectors.h
LII_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -fstatic-stack -mlto-zp=110
.PHONY: test-lm-integration-init sanitize-lm-integration-init c64-lm-integration-init vice-lm-integration-init lm-integration-init-differential
all: build/test-lm-integration-init
test: test-lm-integration-init
sanitize: sanitize-lm-integration-init
build/test-lm-integration-init: $(LII_SRC) tests/test_lm_integration_init.c $(LII_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LII_SRC) tests/test_lm_integration_init.c -o $@
test-lm-integration-init: build/test-lm-integration-init
	./build/test-lm-integration-init
sanitize-lm-integration-init: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(LII_SRC) tests/test_lm_integration_init.c -o build/test-lm-integration-init-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-integration-init-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the listing build).
lm-integration-init-differential:
	python3 tools/interp_routine_differential.py lm-integration-init --cases 512 --freeze
	git diff --exit-code -- tests/lm_integration_init_vectors.h tests/lm_integration_init_c64_vectors.h tools/mk/lm-integration-init.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
include tools/mk/lm-integration-init.groups
LII_C64_PRGS = $(foreach g,$(LM_INTEGRATION_INIT_GROUPS),build/lm-integration-init-$(g).prg)
LII_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c src/rtb_agreement.c src/lm_integration_moves.c tests/interp_case.c tests/lm_integration_init_target.c
LII_C64_lm = src/lm_integration_init.c src/lm_interp_orders.c src/lm_interp_steps.c
c64-lm-integration-init: $(LII_C64_PRGS)
build/lm-integration-init-%.prg: $(LII_SRC) platform/c64/lm_integration_init_diagnostic.c tools/mk/lm-integration-init.groups $(LII_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(LII_MOS_FLAGS) $(LM_INTEGRATION_INIT_GROUP_FLAGS_$*) -Wl,-Map,build/lm-integration-init-$*.map $(LII_C64_$(LM_INTEGRATION_INIT_GROUP_MISSION_$*)) $(LII_C64_COMMON) platform/c64/lm_integration_init_diagnostic.c -o $@
vice-lm-integration-init: $(LII_C64_PRGS)
	for p in $(LII_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
