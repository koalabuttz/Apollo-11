# SPDX-License-Identifier: MIT
# Milestone cm-p30-p37 (lane E): Comanche055 P30-P37 compiled by
# tools/interp_compile.py into native routines of the control-transfer ABI.
CP30_COMMON = src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
CP30_SRC = src/cm_p30_p37.c tests/cm_p30_p37_target.c $(CP30_COMMON)
CP30_HDR = include/cm_p30_p37.h tests/interp_case.h tests/cm_p30_p37_vectors.h tests/cm_p30_p37_c64_vectors.h
CP30_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror
.PHONY: test-cm-p30-p37 sanitize-cm-p30-p37 c64-cm-p30-p37 vice-cm-p30-p37 cm-p30-p37-differential
all: build/test-cm-p30-p37
test: test-cm-p30-p37
sanitize: sanitize-cm-p30-p37
build/test-cm-p30-p37: $(CP30_SRC) tests/test_cm_p30_p37.c $(CP30_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CP30_SRC) tests/test_cm_p30_p37.c -o $@
test-cm-p30-p37: build/test-cm-p30-p37
	./build/test-cm-p30-p37
sanitize-cm-p30-p37: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CP30_SRC) tests/test_cm_p30_p37.c -o build/test-cm-p30-p37-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-p30-p37-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
cm-p30-p37-differential:
	python3 tools/interp_routine_differential.py cm-p30-p37 --cases 512 --freeze
	git diff --exit-code -- tests/cm_p30_p37_vectors.h tests/cm_p30_p37_c64_vectors.h tools/mk/cm-p30-p37.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
include tools/mk/cm-p30-p37.groups
CP30_C64_PRGS = $(foreach g,$(CM_P30_P37_GROUPS),build/cm-p30-p37-$(g).prg)
CP30_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/cm_p30_p37_target.c
CP30_C64_cm = src/cm_p30_p37.c src/cm_interp_orders.c src/cm_interp_steps.c
c64-cm-p30-p37: $(CP30_C64_PRGS)
build/cm-p30-p37-%.prg: $(CP30_SRC) platform/c64/cm_p30_p37_diagnostic.c tools/mk/cm-p30-p37.groups $(CP30_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CP30_MOS_FLAGS) $(CM_P30_P37_GROUP_FLAGS_$*) -Wl,-Map,build/cm-p30-p37-$*.map $(CP30_C64_$(CM_P30_P37_GROUP_MISSION_$*)) $(CP30_C64_COMMON) platform/c64/cm_p30_p37_diagnostic.c -o $@
vice-cm-p30-p37: $(CP30_C64_PRGS)
	for p in $(CP30_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
