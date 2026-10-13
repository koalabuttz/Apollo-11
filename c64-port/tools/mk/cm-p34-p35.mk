# SPDX-License-Identifier: MIT
# Milestone cm-p34-p35 (lane E): Comanche055 P34-35_P74-75 compiled by
# tools/interp_compile.py into native routines of the control-transfer ABI.
CP34_COMMON = src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c src/rtb_modes.c tests/interp_case.c
CP34_SRC = src/cm_p34_p35.c tests/cm_p34_p35_target.c $(CP34_COMMON)
CP34_HDR = include/cm_p34_p35.h tests/interp_case.h tests/cm_p34_p35_vectors.h tests/cm_p34_p35_c64_vectors.h
CP34_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror
.PHONY: test-cm-p34-p35 sanitize-cm-p34-p35 c64-cm-p34-p35 vice-cm-p34-p35 cm-p34-p35-differential
all: build/test-cm-p34-p35
test: test-cm-p34-p35
sanitize: sanitize-cm-p34-p35
build/test-cm-p34-p35: $(CP34_SRC) tests/test_cm_p34_p35.c $(CP34_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CP34_SRC) tests/test_cm_p34_p35.c -o $@
test-cm-p34-p35: build/test-cm-p34-p35
	./build/test-cm-p34-p35
sanitize-cm-p34-p35: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CP34_SRC) tests/test_cm_p34_p35.c -o build/test-cm-p34-p35-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-p34-p35-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
cm-p34-p35-differential:
	python3 tools/interp_routine_differential.py cm-p34-p35 --cases 512 --freeze
	git diff --exit-code -- tests/cm_p34_p35_vectors.h tests/cm_p34_p35_c64_vectors.h tools/mk/cm-p34-p35.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
include tools/mk/cm-p34-p35.groups
CP34_C64_PRGS = $(foreach g,$(CM_P34_P35_GROUPS),build/cm-p34-p35-$(g).prg)
CP34_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c src/rtb_modes.c tests/interp_case.c tests/cm_p34_p35_target.c
CP34_C64_cm = src/cm_p34_p35.c src/cm_interp_orders.c src/cm_interp_steps.c
c64-cm-p34-p35: $(CP34_C64_PRGS)
build/cm-p34-p35-%.prg: $(CP34_SRC) platform/c64/cm_p34_p35_diagnostic.c tools/mk/cm-p34-p35.groups $(CP34_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CP34_MOS_FLAGS) $(CM_P34_P35_GROUP_FLAGS_$*) -Wl,-Map,build/cm-p34-p35-$*.map $(CP34_C64_$(CM_P34_P35_GROUP_MISSION_$*)) $(CP34_C64_COMMON) platform/c64/cm_p34_p35_diagnostic.c -o $@
vice-cm-p34-p35: $(CP34_C64_PRGS)
	for p in $(CP34_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
