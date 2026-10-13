# SPDX-License-Identifier: MIT
# Milestone lm-p34-p35 (lane E): Luminary099 P34-35_P74-75 compiled by
# tools/interp_compile.py into native routines of the control-transfer ABI.
LP34_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c src/rtb_modes.c tests/interp_case.c
LP34_SRC = src/lm_p34_p35.c tests/lm_p34_p35_target.c $(LP34_COMMON)
LP34_HDR = include/lm_p34_p35.h tests/interp_case.h tests/lm_p34_p35_vectors.h tests/lm_p34_p35_c64_vectors.h
LP34_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror
.PHONY: test-lm-p34-p35 sanitize-lm-p34-p35 c64-lm-p34-p35 vice-lm-p34-p35 lm-p34-p35-differential
all: build/test-lm-p34-p35
test: test-lm-p34-p35
sanitize: sanitize-lm-p34-p35
build/test-lm-p34-p35: $(LP34_SRC) tests/test_lm_p34_p35.c $(LP34_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LP34_SRC) tests/test_lm_p34_p35.c -o $@
test-lm-p34-p35: build/test-lm-p34-p35
	./build/test-lm-p34-p35
sanitize-lm-p34-p35: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(LP34_SRC) tests/test_lm_p34_p35.c -o build/test-lm-p34-p35-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-p34-p35-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
lm-p34-p35-differential:
	python3 tools/interp_routine_differential.py lm-p34-p35 --cases 512 --freeze
	git diff --exit-code -- tests/lm_p34_p35_vectors.h tests/lm_p34_p35_c64_vectors.h tools/mk/lm-p34-p35.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
include tools/mk/lm-p34-p35.groups
LP34_C64_PRGS = $(foreach g,$(LM_P34_P35_GROUPS),build/lm-p34-p35-$(g).prg)
LP34_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c src/rtb_modes.c tests/interp_case.c tests/lm_p34_p35_target.c
LP34_C64_lm = src/lm_p34_p35.c src/lm_interp_orders.c src/lm_interp_steps.c
c64-lm-p34-p35: $(LP34_C64_PRGS)
build/lm-p34-p35-%.prg: $(LP34_SRC) platform/c64/lm_p34_p35_diagnostic.c tools/mk/lm-p34-p35.groups $(LP34_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(LP34_MOS_FLAGS) $(LM_P34_P35_GROUP_FLAGS_$*) -Wl,-Map,build/lm-p34-p35-$*.map $(LP34_C64_$(LM_P34_P35_GROUP_MISSION_$*)) $(LP34_C64_COMMON) platform/c64/lm_p34_p35_diagnostic.c -o $@
vice-lm-p34-p35: $(LP34_C64_PRGS)
	for p in $(LP34_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
