# SPDX-License-Identifier: MIT
# Milestone cm-integration-init (lane G): Comanche055 INTEGRATION_INITIALIZATION compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
CM_INTEGRATION_INIT_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
CM_INTEGRATION_INIT_SRC = src/cm_integration_init.c tests/cm_integration_init_target.c $(CM_INTEGRATION_INIT_COMMON)
CM_INTEGRATION_INIT_HDR = include/cm_integration_init.h tests/interp_case.h tests/cm_integration_init_vectors.h tests/cm_integration_init_c64_vectors.h
CM_INTEGRATION_INIT_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -fstatic-stack -mlto-zp=110 -DIRC_CHANNELS
.PHONY: test-cm-integration-init sanitize-cm-integration-init c64-cm-integration-init vice-cm-integration-init cm-integration-init-differential
all: build/test-cm-integration-init
test: test-cm-integration-init
sanitize: sanitize-cm-integration-init
build/test-cm-integration-init: $(CM_INTEGRATION_INIT_SRC) tests/test_cm_integration_init.c $(CM_INTEGRATION_INIT_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_INTEGRATION_INIT_SRC) tests/test_cm_integration_init.c -o $@
test-cm-integration-init: build/test-cm-integration-init
	./build/test-cm-integration-init
sanitize-cm-integration-init: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_INTEGRATION_INIT_SRC) tests/test_cm_integration_init.c -o build/test-cm-integration-init-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-integration-init-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Comanche055 listing build).
cm-integration-init-differential:
	python3 tools/interp_routine_differential.py cm-integration-init --cases 256 --freeze
	git diff --exit-code -- tests/cm_integration_init_vectors.h tests/cm_integration_init_c64_vectors.h tools/mk/cm-integration-init.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/cm-integration-init.groups
CM_INTEGRATION_INIT_C64_PRGS = $(foreach g,$(CM_INTEGRATION_INIT_GROUPS),build/cm-integration-init-$(g).prg)
CM_INTEGRATION_INIT_C64_SRC = src/cm_integration_init.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/cm_integration_init_target.c
c64-cm-integration-init: $(CM_INTEGRATION_INIT_C64_PRGS)
build/cm-integration-init-%.prg: $(CM_INTEGRATION_INIT_SRC) platform/c64/cm_integration_init_diagnostic.c tools/mk/cm-integration-init.groups $(CM_INTEGRATION_INIT_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CM_INTEGRATION_INIT_MOS_FLAGS) $(CM_INTEGRATION_INIT_GROUP_FLAGS_$*) -Wl,-Map,build/cm-integration-init-$*.map $(CM_INTEGRATION_INIT_C64_SRC) platform/c64/cm_integration_init_diagnostic.c -o $@
vice-cm-integration-init: $(CM_INTEGRATION_INIT_C64_PRGS)
	for p in $(CM_INTEGRATION_INIT_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
