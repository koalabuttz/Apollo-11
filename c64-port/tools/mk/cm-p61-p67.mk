# SPDX-License-Identifier: MIT
# Milestone cm-p61-p67 (lane G): Comanche055 P61-P67 compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
CM_P61_P67_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c
CM_P61_P67_SRC = src/cm_p61_p67.c tests/cm_p61_p67_target.c $(CM_P61_P67_COMMON)
CM_P61_P67_HDR = include/cm_p61_p67.h tests/interp_case.h tests/cm_p61_p67_vectors.h tests/cm_p61_p67_c64_vectors.h
CM_P61_P67_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -DIRC_CHANNELS
.PHONY: test-cm-p61-p67 sanitize-cm-p61-p67 c64-cm-p61-p67 vice-cm-p61-p67 cm-p61-p67-differential
all: build/test-cm-p61-p67
test: test-cm-p61-p67
sanitize: sanitize-cm-p61-p67
build/test-cm-p61-p67: $(CM_P61_P67_SRC) tests/test_cm_p61_p67.c $(CM_P61_P67_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_P61_P67_SRC) tests/test_cm_p61_p67.c -o $@
test-cm-p61-p67: build/test-cm-p61-p67
	./build/test-cm-p61-p67
sanitize-cm-p61-p67: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_P61_P67_SRC) tests/test_cm_p61_p67.c -o build/test-cm-p61-p67-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-p61-p67-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Comanche055 listing build).
cm-p61-p67-differential:
	python3 tools/interp_routine_differential.py cm-p61-p67 --cases 256 --freeze
	git diff --exit-code -- tests/cm_p61_p67_vectors.h tests/cm_p61_p67_c64_vectors.h tools/mk/cm-p61-p67.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/cm-p61-p67.groups
CM_P61_P67_C64_PRGS = $(foreach g,$(CM_P61_P67_GROUPS),build/cm-p61-p67-$(g).prg)
CM_P61_P67_C64_SRC = src/cm_p61_p67.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c tests/cm_p61_p67_target.c
c64-cm-p61-p67: $(CM_P61_P67_C64_PRGS)
build/cm-p61-p67-%.prg: $(CM_P61_P67_SRC) platform/c64/cm_p61_p67_diagnostic.c tools/mk/cm-p61-p67.groups $(CM_P61_P67_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CM_P61_P67_MOS_FLAGS) $(CM_P61_P67_GROUP_FLAGS_$*) -Wl,-Map,build/cm-p61-p67-$*.map $(CM_P61_P67_C64_SRC) platform/c64/cm_p61_p67_diagnostic.c -o $@
vice-cm-p61-p67: $(CM_P61_P67_C64_PRGS)
	for p in $(CM_P61_P67_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
