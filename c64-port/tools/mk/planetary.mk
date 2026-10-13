# SPDX-License-Identifier: MIT
# Milestone planetary (lane A2): Luminary099 and Comanche055 PLANETARY_INERTIAL_ORIENTATION compiled by
# tools/interp_compile.py into native routines of the control-transfer ABI.
PLANETARY_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
PLANETARY_SRC = src/lm_planetary.c src/cm_planetary.c tests/planetary_target.c $(PLANETARY_COMMON)
PLANETARY_HDR = include/lm_planetary.h include/cm_planetary.h tests/interp_case.h tests/planetary_vectors.h tests/planetary_c64_vectors.h
PLANETARY_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -fstatic-stack -mlto-zp=110
.PHONY: test-planetary sanitize-planetary c64-planetary vice-planetary planetary-differential
all: build/test-planetary
test: test-planetary
sanitize: sanitize-planetary
build/test-planetary: $(PLANETARY_SRC) tests/test_planetary.c $(PLANETARY_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(PLANETARY_SRC) tests/test_planetary.c -o $@
test-planetary: build/test-planetary
	./build/test-planetary
sanitize-planetary: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(PLANETARY_SRC) tests/test_planetary.c -o build/test-planetary-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-planetary-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
planetary-differential:
	python3 tools/interp_routine_differential.py planetary --cases 512 --freeze
	git diff --exit-code -- tests/planetary_vectors.h tests/planetary_c64_vectors.h tools/mk/planetary.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
include tools/mk/planetary.groups
PLANETARY_C64_PRGS = $(foreach g,$(PLANETARY_GROUPS),build/planetary-$(g).prg)
PLANETARY_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/planetary_target.c
PLANETARY_C64_lm = src/lm_planetary.c src/lm_interp_orders.c src/lm_interp_steps.c
PLANETARY_C64_cm = src/cm_planetary.c src/cm_interp_orders.c src/cm_interp_steps.c
c64-planetary: $(PLANETARY_C64_PRGS)
build/planetary-%.prg: $(PLANETARY_SRC) platform/c64/planetary_diagnostic.c tools/mk/planetary.groups $(PLANETARY_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(PLANETARY_MOS_FLAGS) $(PLANETARY_GROUP_FLAGS_$*) -Wl,-Map,build/planetary-$*.map $(PLANETARY_C64_$(PLANETARY_GROUP_MISSION_$*)) $(PLANETARY_C64_COMMON) platform/c64/planetary_diagnostic.c -o $@
vice-planetary: $(PLANETARY_C64_PRGS)
	for p in $(PLANETARY_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
