# SPDX-License-Identifier: MIT
# Milestone cm-t4rupt-optics (lane A): the Comanche055 T4RUPT_PROGRAM optics monitor and drive compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
CM_T4RUPT_OPTICS_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
CM_T4RUPT_OPTICS_SRC = src/cm_t4rupt_optics.c tests/cm_t4rupt_optics_target.c $(CM_T4RUPT_OPTICS_COMMON)
CM_T4RUPT_OPTICS_HDR = include/cm_t4rupt_optics.h tests/interp_case.h tests/cm_t4rupt_optics_vectors.h tests/cm_t4rupt_optics_c64_vectors.h
CM_T4RUPT_OPTICS_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -DIRC_CHANNELS -DAGC_CHANNELS_HW=irc_cm_hw
.PHONY: test-cm-t4rupt-optics sanitize-cm-t4rupt-optics c64-cm-t4rupt-optics vice-cm-t4rupt-optics cm-t4rupt-optics-differential
all: build/test-cm-t4rupt-optics
test: test-cm-t4rupt-optics
sanitize: sanitize-cm-t4rupt-optics
build/test-cm-t4rupt-optics: $(CM_T4RUPT_OPTICS_SRC) tests/test_cm_t4rupt_optics.c $(CM_T4RUPT_OPTICS_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_T4RUPT_OPTICS_SRC) tests/test_cm_t4rupt_optics.c -o $@
test-cm-t4rupt-optics: build/test-cm-t4rupt-optics
	./build/test-cm-t4rupt-optics
sanitize-cm-t4rupt-optics: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_T4RUPT_OPTICS_SRC) tests/test_cm_t4rupt_optics.c -o build/test-cm-t4rupt-optics-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-t4rupt-optics-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Comanche055 listing build).
cm-t4rupt-optics-differential:
	python3 tools/interp_routine_differential.py cm-t4rupt-optics --cases 512 --freeze
	git diff --exit-code -- tests/cm_t4rupt_optics_vectors.h tests/cm_t4rupt_optics_c64_vectors.h tools/mk/cm-t4rupt-optics.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/cm-t4rupt-optics.groups
CM_T4RUPT_OPTICS_C64_PRGS = $(foreach g,$(CM_T4RUPT_OPTICS_GROUPS),build/cm-t4rupt-optics-$(g).prg)
CM_T4RUPT_OPTICS_C64_SRC = src/cm_t4rupt_optics.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/cm_t4rupt_optics_target.c
c64-cm-t4rupt-optics: $(CM_T4RUPT_OPTICS_C64_PRGS)
build/cm-t4rupt-optics-%.prg: $(CM_T4RUPT_OPTICS_SRC) platform/c64/cm_t4rupt_optics_diagnostic.c tools/mk/cm-t4rupt-optics.groups $(CM_T4RUPT_OPTICS_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CM_T4RUPT_OPTICS_MOS_FLAGS) $(CM_T4RUPT_OPTICS_GROUP_FLAGS_$*) -Wl,-Map,build/cm-t4rupt-optics-$*.map $(CM_T4RUPT_OPTICS_C64_SRC) platform/c64/cm_t4rupt_optics_diagnostic.c -o $@
vice-cm-t4rupt-optics: $(CM_T4RUPT_OPTICS_C64_PRGS)
	for p in $(CM_T4RUPT_OPTICS_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
