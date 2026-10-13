# SPDX-License-Identifier: MIT
# Milestone cm-tvc-restarts (lane G): Comanche055 TVCRESTARTS compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
CM_TVC_RESTARTS_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
CM_TVC_RESTARTS_SRC = src/cm_tvc_restarts.c tests/cm_tvc_restarts_target.c $(CM_TVC_RESTARTS_COMMON)
CM_TVC_RESTARTS_HDR = include/cm_tvc_restarts.h tests/interp_case.h tests/cm_tvc_restarts_vectors.h tests/cm_tvc_restarts_c64_vectors.h
CM_TVC_RESTARTS_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -DIRC_CHANNELS
.PHONY: test-cm-tvc-restarts sanitize-cm-tvc-restarts c64-cm-tvc-restarts vice-cm-tvc-restarts cm-tvc-restarts-differential
all: build/test-cm-tvc-restarts
test: test-cm-tvc-restarts
sanitize: sanitize-cm-tvc-restarts
build/test-cm-tvc-restarts: $(CM_TVC_RESTARTS_SRC) tests/test_cm_tvc_restarts.c $(CM_TVC_RESTARTS_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_TVC_RESTARTS_SRC) tests/test_cm_tvc_restarts.c -o $@
test-cm-tvc-restarts: build/test-cm-tvc-restarts
	./build/test-cm-tvc-restarts
sanitize-cm-tvc-restarts: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_TVC_RESTARTS_SRC) tests/test_cm_tvc_restarts.c -o build/test-cm-tvc-restarts-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-tvc-restarts-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Comanche055 listing build).
cm-tvc-restarts-differential:
	python3 tools/interp_routine_differential.py cm-tvc-restarts --cases 256 --freeze
	git diff --exit-code -- tests/cm_tvc_restarts_vectors.h tests/cm_tvc_restarts_c64_vectors.h tools/mk/cm-tvc-restarts.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/cm-tvc-restarts.groups
CM_TVC_RESTARTS_C64_PRGS = $(foreach g,$(CM_TVC_RESTARTS_GROUPS),build/cm-tvc-restarts-$(g).prg)
CM_TVC_RESTARTS_C64_SRC = src/cm_tvc_restarts.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/cm_tvc_restarts_target.c
c64-cm-tvc-restarts: $(CM_TVC_RESTARTS_C64_PRGS)
build/cm-tvc-restarts-%.prg: $(CM_TVC_RESTARTS_SRC) platform/c64/cm_tvc_restarts_diagnostic.c tools/mk/cm-tvc-restarts.groups $(CM_TVC_RESTARTS_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CM_TVC_RESTARTS_MOS_FLAGS) $(CM_TVC_RESTARTS_GROUP_FLAGS_$*) -Wl,-Map,build/cm-tvc-restarts-$*.map $(CM_TVC_RESTARTS_C64_SRC) platform/c64/cm_tvc_restarts_diagnostic.c -o $@
vice-cm-tvc-restarts: $(CM_TVC_RESTARTS_C64_PRGS)
	for p in $(CM_TVC_RESTARTS_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
