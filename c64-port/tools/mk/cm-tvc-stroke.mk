# SPDX-License-Identifier: MIT
# Milestone cm-tvc-stroke (lane G): Comanche055 TVCSTROKETEST compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
CM_TVC_STROKE_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
CM_TVC_STROKE_SRC = src/cm_tvc_stroke.c tests/cm_tvc_stroke_target.c $(CM_TVC_STROKE_COMMON)
CM_TVC_STROKE_HDR = include/cm_tvc_stroke.h tests/interp_case.h tests/cm_tvc_stroke_vectors.h tests/cm_tvc_stroke_c64_vectors.h
CM_TVC_STROKE_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline
.PHONY: test-cm-tvc-stroke sanitize-cm-tvc-stroke c64-cm-tvc-stroke vice-cm-tvc-stroke cm-tvc-stroke-differential
all: build/test-cm-tvc-stroke
test: test-cm-tvc-stroke
sanitize: sanitize-cm-tvc-stroke
build/test-cm-tvc-stroke: $(CM_TVC_STROKE_SRC) tests/test_cm_tvc_stroke.c $(CM_TVC_STROKE_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_TVC_STROKE_SRC) tests/test_cm_tvc_stroke.c -o $@
test-cm-tvc-stroke: build/test-cm-tvc-stroke
	./build/test-cm-tvc-stroke
sanitize-cm-tvc-stroke: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_TVC_STROKE_SRC) tests/test_cm_tvc_stroke.c -o build/test-cm-tvc-stroke-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-tvc-stroke-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Comanche055 listing build).
cm-tvc-stroke-differential:
	python3 tools/interp_routine_differential.py cm-tvc-stroke --cases 256 --freeze
	git diff --exit-code -- tests/cm_tvc_stroke_vectors.h tests/cm_tvc_stroke_c64_vectors.h tools/mk/cm-tvc-stroke.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/cm-tvc-stroke.groups
CM_TVC_STROKE_C64_PRGS = $(foreach g,$(CM_TVC_STROKE_GROUPS),build/cm-tvc-stroke-$(g).prg)
CM_TVC_STROKE_C64_SRC = src/cm_tvc_stroke.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/cm_tvc_stroke_target.c
c64-cm-tvc-stroke: $(CM_TVC_STROKE_C64_PRGS)
build/cm-tvc-stroke-%.prg: $(CM_TVC_STROKE_SRC) platform/c64/cm_tvc_stroke_diagnostic.c tools/mk/cm-tvc-stroke.groups $(CM_TVC_STROKE_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CM_TVC_STROKE_MOS_FLAGS) $(CM_TVC_STROKE_GROUP_FLAGS_$*) -Wl,-Map,build/cm-tvc-stroke-$*.map $(CM_TVC_STROKE_C64_SRC) platform/c64/cm_tvc_stroke_diagnostic.c -o $@
vice-cm-tvc-stroke: $(CM_TVC_STROKE_C64_PRGS)
	for p in $(CM_TVC_STROKE_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
