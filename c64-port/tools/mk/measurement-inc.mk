# SPDX-License-Identifier: MIT
# Milestone measurement-inc (lane D2): Luminary099 and Comanche055
# MEASUREMENT_INCORPORATION (both missions) compiled by tools/interp_compile.py into
# native routines of the control-transfer ABI.
MINC_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c src/rtb_modes.c src/lm_integration_moves.c src/cm_integration_moves.c tests/interp_case.c
MINC_SRC = src/lm_measurement_inc.c src/cm_measurement_inc.c tests/measurement_inc_target.c $(MINC_COMMON)
MINC_HDR = include/lm_measurement_inc.h include/cm_measurement_inc.h tests/interp_case.h tests/measurement_inc_vectors.h tests/measurement_inc_c64_vectors.h
MINC_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror
.PHONY: test-measurement-inc sanitize-measurement-inc c64-measurement-inc vice-measurement-inc measurement-inc-differential
all: build/test-measurement-inc
test: test-measurement-inc
sanitize: sanitize-measurement-inc
build/test-measurement-inc: $(MINC_SRC) tests/test_measurement_inc.c $(MINC_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(MINC_SRC) tests/test_measurement_inc.c -o $@
test-measurement-inc: build/test-measurement-inc
	./build/test-measurement-inc
sanitize-measurement-inc: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(MINC_SRC) tests/test_measurement_inc.c -o build/test-measurement-inc-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-measurement-inc-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
measurement-inc-differential:
	python3 tools/interp_routine_differential.py measurement-inc --cases 512 --freeze
	git diff --exit-code -- tests/measurement_inc_vectors.h tests/measurement_inc_c64_vectors.h tools/mk/measurement-inc.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
include tools/mk/measurement-inc.groups
MINC_C64_PRGS = $(foreach g,$(MEASUREMENT_INC_GROUPS),build/measurement-inc-$(g).prg)
MINC_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c src/rtb_modes.c src/lm_integration_moves.c src/cm_integration_moves.c tests/interp_case.c tests/measurement_inc_target.c
MINC_C64_lm = src/lm_measurement_inc.c src/lm_interp_orders.c src/lm_interp_steps.c
MINC_C64_cm = src/cm_measurement_inc.c src/cm_interp_orders.c src/cm_interp_steps.c
c64-measurement-inc: $(MINC_C64_PRGS)
build/measurement-inc-%.prg: $(MINC_SRC) platform/c64/measurement_inc_diagnostic.c tools/mk/measurement-inc.groups $(MINC_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(MINC_MOS_FLAGS) $(MEASUREMENT_INC_GROUP_FLAGS_$*) -Wl,-Map,build/measurement-inc-$*.map $(MINC_C64_$(MEASUREMENT_INC_GROUP_MISSION_$*)) $(MINC_C64_COMMON) platform/c64/measurement_inc_diagnostic.c -o $@
vice-measurement-inc: $(MINC_C64_PRGS)
	for p in $(MINC_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
