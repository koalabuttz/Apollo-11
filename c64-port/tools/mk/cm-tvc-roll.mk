# SPDX-License-Identifier: MIT
# Milestone cm-tvc-roll (lane G): Comanche055 TVCROLLDAP compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
CM_TVC_ROLL_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
CM_TVC_ROLL_SRC = src/cm_tvc_roll.c tests/cm_tvc_roll_target.c $(CM_TVC_ROLL_COMMON)
CM_TVC_ROLL_HDR = include/cm_tvc_roll.h tests/interp_case.h tests/cm_tvc_roll_vectors.h tests/cm_tvc_roll_c64_vectors.h
CM_TVC_ROLL_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -DIRC_CHANNELS -DAGC_CHANNELS_HW=irc_cm_hw
.PHONY: test-cm-tvc-roll sanitize-cm-tvc-roll c64-cm-tvc-roll vice-cm-tvc-roll cm-tvc-roll-differential
all: build/test-cm-tvc-roll
test: test-cm-tvc-roll
sanitize: sanitize-cm-tvc-roll
build/test-cm-tvc-roll: $(CM_TVC_ROLL_SRC) tests/test_cm_tvc_roll.c $(CM_TVC_ROLL_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_TVC_ROLL_SRC) tests/test_cm_tvc_roll.c -o $@
test-cm-tvc-roll: build/test-cm-tvc-roll
	./build/test-cm-tvc-roll
sanitize-cm-tvc-roll: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_TVC_ROLL_SRC) tests/test_cm_tvc_roll.c -o build/test-cm-tvc-roll-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-tvc-roll-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Comanche055 listing build).
cm-tvc-roll-differential:
	python3 tools/interp_routine_differential.py cm-tvc-roll --cases 256 --freeze
	git diff --exit-code -- tests/cm_tvc_roll_vectors.h tests/cm_tvc_roll_c64_vectors.h tools/mk/cm-tvc-roll.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/cm-tvc-roll.groups
CM_TVC_ROLL_C64_PRGS = $(foreach g,$(CM_TVC_ROLL_GROUPS),build/cm-tvc-roll-$(g).prg)
CM_TVC_ROLL_C64_SRC = src/cm_tvc_roll.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/cm_tvc_roll_target.c
c64-cm-tvc-roll: $(CM_TVC_ROLL_C64_PRGS)
build/cm-tvc-roll-%.prg: $(CM_TVC_ROLL_SRC) platform/c64/cm_tvc_roll_diagnostic.c tools/mk/cm-tvc-roll.groups $(CM_TVC_ROLL_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CM_TVC_ROLL_MOS_FLAGS) $(CM_TVC_ROLL_GROUP_FLAGS_$*) -Wl,-Map,build/cm-tvc-roll-$*.map $(CM_TVC_ROLL_C64_SRC) platform/c64/cm_tvc_roll_diagnostic.c -o $@
vice-cm-tvc-roll: $(CM_TVC_ROLL_C64_PRGS)
	for p in $(CM_TVC_ROLL_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
