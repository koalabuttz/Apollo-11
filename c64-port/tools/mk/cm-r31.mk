# SPDX-License-Identifier: MIT
# Milestone cm-r31 (lane E): Comanche055 R31 compiled by
# tools/interp_compile.py into native routines of the control-transfer ABI.
CR31_COMMON = src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c
CR31_SRC = src/cm_r31.c tests/cm_r31_target.c $(CR31_COMMON)
CR31_HDR = include/cm_r31.h tests/interp_case.h tests/cm_r31_vectors.h tests/cm_r31_c64_vectors.h
CR31_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror
.PHONY: test-cm-r31 sanitize-cm-r31 c64-cm-r31 vice-cm-r31 cm-r31-differential
all: build/test-cm-r31
test: test-cm-r31
sanitize: sanitize-cm-r31
build/test-cm-r31: $(CR31_SRC) tests/test_cm_r31.c $(CR31_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CR31_SRC) tests/test_cm_r31.c -o $@
test-cm-r31: build/test-cm-r31
	./build/test-cm-r31
sanitize-cm-r31: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CR31_SRC) tests/test_cm_r31.c -o build/test-cm-r31-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-r31-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
cm-r31-differential:
	python3 tools/interp_routine_differential.py cm-r31 --cases 512 --freeze
	git diff --exit-code -- tests/cm_r31_vectors.h tests/cm_r31_c64_vectors.h tools/mk/cm-r31.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
include tools/mk/cm-r31.groups
CR31_C64_PRGS = $(foreach g,$(CM_R31_GROUPS),build/cm-r31-$(g).prg)
CR31_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c tests/cm_r31_target.c
CR31_C64_cm = src/cm_r31.c src/cm_interp_orders.c src/cm_interp_steps.c
c64-cm-r31: $(CR31_C64_PRGS)
build/cm-r31-%.prg: $(CR31_SRC) platform/c64/cm_r31_diagnostic.c tools/mk/cm-r31.groups $(CR31_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CR31_MOS_FLAGS) $(CM_R31_GROUP_FLAGS_$*) -Wl,-Map,build/cm-r31-$*.map $(CR31_C64_$(CM_R31_GROUP_MISSION_$*)) $(CR31_C64_COMMON) platform/c64/cm_r31_diagnostic.c -o $@
vice-cm-r31: $(CR31_C64_PRGS)
	for p in $(CR31_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
