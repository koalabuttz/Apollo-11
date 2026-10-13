# SPDX-License-Identifier: MIT
# Milestone cm-p32-p33 (lane E): Comanche055 P32-P33_P72-P73 compiled by
# tools/interp_compile.py into native routines of the control-transfer ABI.
CP32_COMMON = src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c
CP32_SRC = src/cm_p32_p33.c tests/cm_p32_p33_target.c $(CP32_COMMON)
CP32_HDR = include/cm_p32_p33.h tests/interp_case.h tests/cm_p32_p33_vectors.h tests/cm_p32_p33_c64_vectors.h
CP32_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror
.PHONY: test-cm-p32-p33 sanitize-cm-p32-p33 c64-cm-p32-p33 vice-cm-p32-p33 cm-p32-p33-differential
all: build/test-cm-p32-p33
test: test-cm-p32-p33
sanitize: sanitize-cm-p32-p33
build/test-cm-p32-p33: $(CP32_SRC) tests/test_cm_p32_p33.c $(CP32_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CP32_SRC) tests/test_cm_p32_p33.c -o $@
test-cm-p32-p33: build/test-cm-p32-p33
	./build/test-cm-p32-p33
sanitize-cm-p32-p33: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CP32_SRC) tests/test_cm_p32_p33.c -o build/test-cm-p32-p33-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-p32-p33-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
cm-p32-p33-differential:
	python3 tools/interp_routine_differential.py cm-p32-p33 --cases 512 --freeze
	git diff --exit-code -- tests/cm_p32_p33_vectors.h tests/cm_p32_p33_c64_vectors.h tools/mk/cm-p32-p33.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
include tools/mk/cm-p32-p33.groups
CP32_C64_PRGS = $(foreach g,$(CM_P32_P33_GROUPS),build/cm-p32-p33-$(g).prg)
CP32_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c tests/cm_p32_p33_target.c
CP32_C64_cm = src/cm_p32_p33.c src/cm_interp_orders.c src/cm_interp_steps.c
c64-cm-p32-p33: $(CP32_C64_PRGS)
build/cm-p32-p33-%.prg: $(CP32_SRC) platform/c64/cm_p32_p33_diagnostic.c tools/mk/cm-p32-p33.groups $(CP32_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CP32_MOS_FLAGS) $(CM_P32_P33_GROUP_FLAGS_$*) -Wl,-Map,build/cm-p32-p33-$*.map $(CP32_C64_$(CM_P32_P33_GROUP_MISSION_$*)) $(CP32_C64_COMMON) platform/c64/cm_p32_p33_diagnostic.c -o $@
vice-cm-p32-p33: $(CP32_C64_PRGS)
	for p in $(CP32_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
