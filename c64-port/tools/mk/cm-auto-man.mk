# SPDX-License-Identifier: MIT
# Milestone cm-auto-man (lane G): Comanche055 AUTOMATIC_MANEUVERS compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
CM_AUTO_MAN_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
CM_AUTO_MAN_SRC = src/cm_auto_man.c tests/cm_auto_man_target.c $(CM_AUTO_MAN_COMMON)
CM_AUTO_MAN_HDR = include/cm_auto_man.h tests/interp_case.h tests/cm_auto_man_vectors.h tests/cm_auto_man_c64_vectors.h
CM_AUTO_MAN_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -DIRC_CHANNELS -DAGC_CHANNELS_HW=irc_cm_hw
.PHONY: test-cm-auto-man sanitize-cm-auto-man c64-cm-auto-man vice-cm-auto-man cm-auto-man-differential
all: build/test-cm-auto-man
test: test-cm-auto-man
sanitize: sanitize-cm-auto-man
build/test-cm-auto-man: $(CM_AUTO_MAN_SRC) tests/test_cm_auto_man.c $(CM_AUTO_MAN_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_AUTO_MAN_SRC) tests/test_cm_auto_man.c -o $@
test-cm-auto-man: build/test-cm-auto-man
	./build/test-cm-auto-man
sanitize-cm-auto-man: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_AUTO_MAN_SRC) tests/test_cm_auto_man.c -o build/test-cm-auto-man-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-auto-man-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Comanche055 listing build).
cm-auto-man-differential:
	python3 tools/interp_routine_differential.py cm-auto-man --cases 256 --freeze
	git diff --exit-code -- tests/cm_auto_man_vectors.h tests/cm_auto_man_c64_vectors.h tools/mk/cm-auto-man.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/cm-auto-man.groups
CM_AUTO_MAN_C64_PRGS = $(foreach g,$(CM_AUTO_MAN_GROUPS),build/cm-auto-man-$(g).prg)
CM_AUTO_MAN_C64_SRC = src/cm_auto_man.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/cm_auto_man_target.c
c64-cm-auto-man: $(CM_AUTO_MAN_C64_PRGS)
build/cm-auto-man-%.prg: $(CM_AUTO_MAN_SRC) platform/c64/cm_auto_man_diagnostic.c tools/mk/cm-auto-man.groups $(CM_AUTO_MAN_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CM_AUTO_MAN_MOS_FLAGS) $(CM_AUTO_MAN_GROUP_FLAGS_$*) -Wl,-Map,build/cm-auto-man-$*.map $(CM_AUTO_MAN_C64_SRC) platform/c64/cm_auto_man_diagnostic.c -o $@
vice-cm-auto-man: $(CM_AUTO_MAN_C64_PRGS)
	for p in $(CM_AUTO_MAN_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
