# SPDX-License-Identifier: MIT
# Milestone cm-p37-p70 (lane E): Comanche055 P37_P70 compiled by
# tools/interp_compile.py into native routines of the control-transfer ABI.
CP37_COMMON = src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c src/rtb_modes.c src/cm_integration_moves.c tests/interp_case.c
CP37_SRC = src/cm_p37_p70.c tests/cm_p37_p70_target.c $(CP37_COMMON)
CP37_HDR = include/cm_p37_p70.h tests/interp_case.h tests/cm_p37_p70_vectors.h tests/cm_p37_p70_c64_vectors.h
CP37_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror
.PHONY: test-cm-p37-p70 sanitize-cm-p37-p70 c64-cm-p37-p70 vice-cm-p37-p70 cm-p37-p70-differential
all: build/test-cm-p37-p70
test: test-cm-p37-p70
sanitize: sanitize-cm-p37-p70
build/test-cm-p37-p70: $(CP37_SRC) tests/test_cm_p37_p70.c $(CP37_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CP37_SRC) tests/test_cm_p37_p70.c -o $@
test-cm-p37-p70: build/test-cm-p37-p70
	./build/test-cm-p37-p70
sanitize-cm-p37-p70: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CP37_SRC) tests/test_cm_p37_p70.c -o build/test-cm-p37-p70-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-p37-p70-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
cm-p37-p70-differential:
	python3 tools/interp_routine_differential.py cm-p37-p70 --cases 512 --freeze
	git diff --exit-code -- tests/cm_p37_p70_vectors.h tests/cm_p37_p70_c64_vectors.h tools/mk/cm-p37-p70.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
include tools/mk/cm-p37-p70.groups
CP37_C64_PRGS = $(foreach g,$(CM_P37_P70_GROUPS),build/cm-p37-p70-$(g).prg)
CP37_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c src/rtb_modes.c src/cm_integration_moves.c tests/interp_case.c tests/interp_cut.c tests/cm_p37_p70_target.c
CP37_C64_cm = src/cm_p37_p70.c src/cm_interp_orders.c src/cm_interp_steps.c
c64-cm-p37-p70: $(CP37_C64_PRGS)
build/cm-p37-p70-%.prg: $(CP37_SRC) platform/c64/cm_p37_p70_diagnostic.c tools/mk/cm-p37-p70.groups $(CP37_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CP37_MOS_FLAGS) $(CM_P37_P70_GROUP_FLAGS_$*) -Wl,-Map,build/cm-p37-p70-$*.map $(CP37_C64_$(CM_P37_P70_GROUP_MISSION_$*)) $(CP37_C64_COMMON) platform/c64/cm_p37_p70_diagnostic.c -o $@
vice-cm-p37-p70: $(CP37_C64_PRGS)
	for p in $(CP37_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
