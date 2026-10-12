# SPDX-License-Identifier: MIT
# Milestone cm-tvc-init (lane G): Comanche055 TVCINITIALIZE compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
CM_TVC_INIT_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
CM_TVC_INIT_SRC = src/cm_tvc_init.c tests/cm_tvc_init_target.c $(CM_TVC_INIT_COMMON)
CM_TVC_INIT_HDR = include/cm_tvc_init.h tests/interp_case.h tests/cm_tvc_init_vectors.h tests/cm_tvc_init_c64_vectors.h
CM_TVC_INIT_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline
.PHONY: test-cm-tvc-init sanitize-cm-tvc-init c64-cm-tvc-init vice-cm-tvc-init cm-tvc-init-differential
all: build/test-cm-tvc-init
test: test-cm-tvc-init
sanitize: sanitize-cm-tvc-init
build/test-cm-tvc-init: $(CM_TVC_INIT_SRC) tests/test_cm_tvc_init.c $(CM_TVC_INIT_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_TVC_INIT_SRC) tests/test_cm_tvc_init.c -o $@
test-cm-tvc-init: build/test-cm-tvc-init
	./build/test-cm-tvc-init
sanitize-cm-tvc-init: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_TVC_INIT_SRC) tests/test_cm_tvc_init.c -o build/test-cm-tvc-init-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-tvc-init-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Comanche055 listing build).
cm-tvc-init-differential:
	python3 tools/interp_routine_differential.py cm-tvc-init --cases 256 --freeze
	git diff --exit-code -- tests/cm_tvc_init_vectors.h tests/cm_tvc_init_c64_vectors.h tools/mk/cm-tvc-init.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/cm-tvc-init.groups
CM_TVC_INIT_C64_PRGS = $(foreach g,$(CM_TVC_INIT_GROUPS),build/cm-tvc-init-$(g).prg)
CM_TVC_INIT_C64_SRC = src/cm_tvc_init.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/cm_tvc_init_target.c
c64-cm-tvc-init: $(CM_TVC_INIT_C64_PRGS)
build/cm-tvc-init-%.prg: $(CM_TVC_INIT_SRC) platform/c64/cm_tvc_init_diagnostic.c tools/mk/cm-tvc-init.groups $(CM_TVC_INIT_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CM_TVC_INIT_MOS_FLAGS) $(CM_TVC_INIT_GROUP_FLAGS_$*) -Wl,-Map,build/cm-tvc-init-$*.map $(CM_TVC_INIT_C64_SRC) platform/c64/cm_tvc_init_diagnostic.c -o $@
vice-cm-tvc-init: $(CM_TVC_INIT_C64_PRGS)
	for p in $(CM_TVC_INIT_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
