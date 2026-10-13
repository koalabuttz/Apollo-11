# SPDX-License-Identifier: MIT
# Milestone cm-conics (lane G): Comanche055 CM_CONICS compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
CM_CONICS_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
CM_CONICS_SRC = src/cm_conics.c tests/cm_conics_target.c $(CM_CONICS_COMMON)
CM_CONICS_HDR = include/cm_conics.h tests/interp_case.h tests/cm_conics_vectors.h tests/cm_conics_c64_vectors.h
CM_CONICS_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline
.PHONY: test-cm-conics sanitize-cm-conics c64-cm-conics vice-cm-conics cm-conics-differential
all: build/test-cm-conics
test: test-cm-conics
sanitize: sanitize-cm-conics
build/test-cm-conics: $(CM_CONICS_SRC) tests/test_cm_conics.c $(CM_CONICS_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_CONICS_SRC) tests/test_cm_conics.c -o $@
test-cm-conics: build/test-cm-conics
	./build/test-cm-conics
sanitize-cm-conics: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_CONICS_SRC) tests/test_cm_conics.c -o build/test-cm-conics-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-conics-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Comanche055 listing build).
cm-conics-differential:
	python3 tools/interp_routine_differential.py cm-conics --cases 256 --freeze
	git diff --exit-code -- tests/cm_conics_vectors.h tests/cm_conics_c64_vectors.h tools/mk/cm-conics.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/cm-conics.groups
CM_CONICS_C64_PRGS = $(foreach g,$(CM_CONICS_GROUPS),build/cm-conics-$(g).prg)
CM_CONICS_C64_SRC = src/cm_conics.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/cm_conics_target.c
c64-cm-conics: $(CM_CONICS_C64_PRGS)
build/cm-conics-%.prg: $(CM_CONICS_SRC) platform/c64/cm_conics_diagnostic.c tools/mk/cm-conics.groups $(CM_CONICS_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CM_CONICS_MOS_FLAGS) $(CM_CONICS_GROUP_FLAGS_$*) -Wl,-Map,build/cm-conics-$*.map $(CM_CONICS_C64_SRC) platform/c64/cm_conics_diagnostic.c -o $@
vice-cm-conics: $(CM_CONICS_C64_PRGS)
	for p in $(CM_CONICS_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
