# SPDX-License-Identifier: MIT
# Milestone cm-servicer (lane G): Comanche055 SERVICER207 compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
CM_SERVICER_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
CM_SERVICER_SRC = src/cm_servicer.c tests/cm_servicer_target.c $(CM_SERVICER_COMMON)
CM_SERVICER_HDR = include/cm_servicer.h tests/interp_case.h tests/cm_servicer_vectors.h tests/cm_servicer_c64_vectors.h
CM_SERVICER_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline
.PHONY: test-cm-servicer sanitize-cm-servicer c64-cm-servicer vice-cm-servicer cm-servicer-differential
all: build/test-cm-servicer
test: test-cm-servicer
sanitize: sanitize-cm-servicer
build/test-cm-servicer: $(CM_SERVICER_SRC) tests/test_cm_servicer.c $(CM_SERVICER_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_SERVICER_SRC) tests/test_cm_servicer.c -o $@
test-cm-servicer: build/test-cm-servicer
	./build/test-cm-servicer
sanitize-cm-servicer: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_SERVICER_SRC) tests/test_cm_servicer.c -o build/test-cm-servicer-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-servicer-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Comanche055 listing build).
cm-servicer-differential:
	python3 tools/interp_routine_differential.py cm-servicer --cases 256 --freeze
	git diff --exit-code -- tests/cm_servicer_vectors.h tests/cm_servicer_c64_vectors.h tools/mk/cm-servicer.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/cm-servicer.groups
CM_SERVICER_C64_PRGS = $(foreach g,$(CM_SERVICER_GROUPS),build/cm-servicer-$(g).prg)
CM_SERVICER_C64_SRC = src/cm_servicer.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/cm_servicer_target.c
c64-cm-servicer: $(CM_SERVICER_C64_PRGS)
build/cm-servicer-%.prg: $(CM_SERVICER_SRC) platform/c64/cm_servicer_diagnostic.c tools/mk/cm-servicer.groups $(CM_SERVICER_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CM_SERVICER_MOS_FLAGS) $(CM_SERVICER_GROUP_FLAGS_$*) -Wl,-Map,build/cm-servicer-$*.map $(CM_SERVICER_C64_SRC) platform/c64/cm_servicer_diagnostic.c -o $@
vice-cm-servicer: $(CM_SERVICER_C64_PRGS)
	for p in $(CM_SERVICER_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
