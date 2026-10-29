# SPDX-License-Identifier: MIT
# Milestone cm-p11 (lane A2): Comanche055 P11 compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
CM_P11_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
CM_P11_SRC = src/cm_p11.c src/rtb_angles.c tests/cm_p11_target.c $(CM_P11_COMMON)
CM_P11_HDR = include/cm_p11.h tests/interp_case.h tests/cm_p11_vectors.h tests/cm_p11_c64_vectors.h
CM_P11_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -DIRC_CHANNELS -DAGC_CHANNELS_HW=irc_cm_hw
.PHONY: test-cm-p11 sanitize-cm-p11 c64-cm-p11 vice-cm-p11 cm-p11-differential
all: build/test-cm-p11
test: test-cm-p11
sanitize: sanitize-cm-p11
build/test-cm-p11: $(CM_P11_SRC) tests/test_cm_p11.c $(CM_P11_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_P11_SRC) tests/test_cm_p11.c -o $@
test-cm-p11: build/test-cm-p11
	./build/test-cm-p11
sanitize-cm-p11: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_P11_SRC) tests/test_cm_p11.c -o build/test-cm-p11-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-p11-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Comanche055 listing build).
cm-p11-differential:
	python3 tools/interp_routine_differential.py cm-p11 --cases 256 --freeze
	git diff --exit-code -- tests/cm_p11_vectors.h tests/cm_p11_c64_vectors.h tools/mk/cm-p11.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/cm-p11.groups
CM_P11_C64_PRGS = $(foreach g,$(CM_P11_GROUPS),build/cm-p11-$(g).prg)
CM_P11_C64_SRC = src/cm_p11.c src/rtb_angles.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/cm_p11_target.c
c64-cm-p11: $(CM_P11_C64_PRGS)
build/cm-p11-%.prg: $(CM_P11_SRC) platform/c64/cm_p11_diagnostic.c tools/mk/cm-p11.groups $(CM_P11_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CM_P11_MOS_FLAGS) $(CM_P11_GROUP_FLAGS_$*) -Wl,-Map,build/cm-p11-$*.map $(CM_P11_C64_SRC) platform/c64/cm_p11_diagnostic.c -o $@
vice-cm-p11: $(CM_P11_C64_PRGS)
	for p in $(CM_P11_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
