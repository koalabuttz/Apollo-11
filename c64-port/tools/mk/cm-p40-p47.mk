# SPDX-License-Identifier: MIT
# Milestone cm-p40-p47 (lane G): Comanche055 P40-P47 compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
CM_P40_P47_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c
CM_P40_P47_SRC = src/cm_p40_p47.c tests/cm_p40_p47_target.c $(CM_P40_P47_COMMON)
CM_P40_P47_HDR = include/cm_p40_p47.h tests/interp_case.h tests/cm_p40_p47_vectors.h tests/cm_p40_p47_c64_vectors.h
CM_P40_P47_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline
.PHONY: test-cm-p40-p47 sanitize-cm-p40-p47 c64-cm-p40-p47 vice-cm-p40-p47 cm-p40-p47-differential
all: build/test-cm-p40-p47
test: test-cm-p40-p47
sanitize: sanitize-cm-p40-p47
build/test-cm-p40-p47: $(CM_P40_P47_SRC) tests/test_cm_p40_p47.c $(CM_P40_P47_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_P40_P47_SRC) tests/test_cm_p40_p47.c -o $@
test-cm-p40-p47: build/test-cm-p40-p47
	./build/test-cm-p40-p47
sanitize-cm-p40-p47: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_P40_P47_SRC) tests/test_cm_p40_p47.c -o build/test-cm-p40-p47-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-p40-p47-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Comanche055 listing build).
cm-p40-p47-differential:
	python3 tools/interp_routine_differential.py cm-p40-p47 --cases 256 --freeze
	git diff --exit-code -- tests/cm_p40_p47_vectors.h tests/cm_p40_p47_c64_vectors.h tools/mk/cm-p40-p47.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/cm-p40-p47.groups
CM_P40_P47_C64_PRGS = $(foreach g,$(CM_P40_P47_GROUPS),build/cm-p40-p47-$(g).prg)
CM_P40_P47_C64_SRC = src/cm_p40_p47.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c tests/cm_p40_p47_target.c
c64-cm-p40-p47: $(CM_P40_P47_C64_PRGS)
build/cm-p40-p47-%.prg: $(CM_P40_P47_SRC) platform/c64/cm_p40_p47_diagnostic.c tools/mk/cm-p40-p47.groups $(CM_P40_P47_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CM_P40_P47_MOS_FLAGS) $(CM_P40_P47_GROUP_FLAGS_$*) -Wl,-Map,build/cm-p40-p47-$*.map $(CM_P40_P47_C64_SRC) platform/c64/cm_p40_p47_diagnostic.c -o $@
vice-cm-p40-p47: $(CM_P40_P47_C64_PRGS)
	for p in $(CM_P40_P47_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
