# SPDX-License-Identifier: MIT
# Milestone cm-orbital-integration (lane G): Comanche055 ORBITAL_INTEGRATION compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
CM_ORBITAL_INTEGRATION_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
CM_ORBITAL_INTEGRATION_SRC = src/cm_orbital_integration.c tests/cm_orbital_integration_target.c $(CM_ORBITAL_INTEGRATION_COMMON)
CM_ORBITAL_INTEGRATION_HDR = include/cm_orbital_integration.h tests/interp_case.h tests/cm_orbital_integration_vectors.h tests/cm_orbital_integration_c64_vectors.h
CM_ORBITAL_INTEGRATION_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -fstatic-stack -mlto-zp=110 -DIRC_CHANNELS
.PHONY: test-cm-orbital-integration sanitize-cm-orbital-integration c64-cm-orbital-integration vice-cm-orbital-integration cm-orbital-integration-differential
all: build/test-cm-orbital-integration
test: test-cm-orbital-integration
sanitize: sanitize-cm-orbital-integration
build/test-cm-orbital-integration: $(CM_ORBITAL_INTEGRATION_SRC) tests/test_cm_orbital_integration.c $(CM_ORBITAL_INTEGRATION_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_ORBITAL_INTEGRATION_SRC) tests/test_cm_orbital_integration.c -o $@
test-cm-orbital-integration: build/test-cm-orbital-integration
	./build/test-cm-orbital-integration
sanitize-cm-orbital-integration: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_ORBITAL_INTEGRATION_SRC) tests/test_cm_orbital_integration.c -o build/test-cm-orbital-integration-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-orbital-integration-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Comanche055 listing build).
cm-orbital-integration-differential:
	python3 tools/interp_routine_differential.py cm-orbital-integration --cases 256 --freeze
	git diff --exit-code -- tests/cm_orbital_integration_vectors.h tests/cm_orbital_integration_c64_vectors.h tools/mk/cm-orbital-integration.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/cm-orbital-integration.groups
CM_ORBITAL_INTEGRATION_C64_PRGS = $(foreach g,$(CM_ORBITAL_INTEGRATION_GROUPS),build/cm-orbital-integration-$(g).prg)
CM_ORBITAL_INTEGRATION_C64_SRC = src/cm_orbital_integration.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/interp_cut.c tests/cm_orbital_integration_target.c
c64-cm-orbital-integration: $(CM_ORBITAL_INTEGRATION_C64_PRGS)
build/cm-orbital-integration-%.prg: $(CM_ORBITAL_INTEGRATION_SRC) platform/c64/cm_orbital_integration_diagnostic.c tools/mk/cm-orbital-integration.groups $(CM_ORBITAL_INTEGRATION_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CM_ORBITAL_INTEGRATION_MOS_FLAGS) $(CM_ORBITAL_INTEGRATION_GROUP_FLAGS_$*) -Wl,-Map,build/cm-orbital-integration-$*.map $(CM_ORBITAL_INTEGRATION_C64_SRC) platform/c64/cm_orbital_integration_diagnostic.c -o $@
vice-cm-orbital-integration: $(CM_ORBITAL_INTEGRATION_C64_PRGS)
	for p in $(CM_ORBITAL_INTEGRATION_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
