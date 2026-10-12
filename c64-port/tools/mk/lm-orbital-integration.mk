# SPDX-License-Identifier: MIT
# Milestone lm-orbital-integration (lane B): Luminary099 ORBITAL_INTEGRATION compiled by
# tools/interp_compile.py into native routines of the control-transfer ABI.
LOI_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/lm_integration_moves.c tests/interp_case.c
LOI_SRC = src/lm_orbital_integration.c tests/lm_orbital_integration_target.c $(LOI_COMMON)
LOI_HDR = include/lm_orbital_integration.h tests/interp_case.h tests/lm_orbital_integration_vectors.h tests/lm_orbital_integration_c64_vectors.h
LOI_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -fstatic-stack -mlto-zp=110
.PHONY: test-lm-orbital-integration sanitize-lm-orbital-integration c64-lm-orbital-integration vice-lm-orbital-integration lm-orbital-integration-differential
all: build/test-lm-orbital-integration
test: test-lm-orbital-integration
sanitize: sanitize-lm-orbital-integration
build/test-lm-orbital-integration: $(LOI_SRC) tests/test_lm_orbital_integration.c $(LOI_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LOI_SRC) tests/test_lm_orbital_integration.c -o $@
test-lm-orbital-integration: build/test-lm-orbital-integration
	./build/test-lm-orbital-integration
sanitize-lm-orbital-integration: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(LOI_SRC) tests/test_lm_orbital_integration.c -o build/test-lm-orbital-integration-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-orbital-integration-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the listing build).
lm-orbital-integration-differential:
	python3 tools/interp_routine_differential.py lm-orbital-integration --cases 512 --freeze
	git diff --exit-code -- tests/lm_orbital_integration_vectors.h tests/lm_orbital_integration_c64_vectors.h tools/mk/lm-orbital-integration.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
include tools/mk/lm-orbital-integration.groups
LOI_C64_PRGS = $(foreach g,$(LM_ORBITAL_INTEGRATION_GROUPS),build/lm-orbital-integration-$(g).prg)
LOI_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/lm_integration_moves.c tests/interp_case.c tests/lm_orbital_integration_target.c
LOI_C64_lm = src/lm_orbital_integration.c src/lm_interp_orders.c src/lm_interp_steps.c
c64-lm-orbital-integration: $(LOI_C64_PRGS)
build/lm-orbital-integration-%.prg: $(LOI_SRC) platform/c64/lm_orbital_integration_diagnostic.c tools/mk/lm-orbital-integration.groups $(LOI_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(LOI_MOS_FLAGS) $(LM_ORBITAL_INTEGRATION_GROUP_FLAGS_$*) -Wl,-Map,build/lm-orbital-integration-$*.map $(LOI_C64_$(LM_ORBITAL_INTEGRATION_GROUP_MISSION_$*)) $(LOI_C64_COMMON) platform/c64/lm_orbital_integration_diagnostic.c -o $@
vice-lm-orbital-integration: $(LOI_C64_PRGS)
	for p in $(LOI_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
