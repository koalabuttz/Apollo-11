# SPDX-License-Identifier: MIT
# Milestone kalcmanu-steering (lane D1b): Luminary099 and Comanche055
# KALCMANU_STEERING (both missions) compiled by tools/interp_compile.py into
# native routines of the control-transfer ABI.
KAL_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c
KAL_SRC = src/lm_kalcmanu_steering.c src/cm_kalcmanu_steering.c tests/kalcmanu_steering_target.c $(KAL_COMMON)
KAL_HDR = include/lm_kalcmanu_steering.h include/cm_kalcmanu_steering.h tests/interp_case.h tests/kalcmanu_steering_vectors.h tests/kalcmanu_steering_c64_vectors.h
KAL_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror
.PHONY: test-kalcmanu-steering sanitize-kalcmanu-steering c64-kalcmanu-steering vice-kalcmanu-steering kalcmanu-steering-differential
all: build/test-kalcmanu-steering
test: test-kalcmanu-steering
sanitize: sanitize-kalcmanu-steering
build/test-kalcmanu-steering: $(KAL_SRC) tests/test_kalcmanu_steering.c $(KAL_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(KAL_SRC) tests/test_kalcmanu_steering.c -o $@
test-kalcmanu-steering: build/test-kalcmanu-steering
	./build/test-kalcmanu-steering
sanitize-kalcmanu-steering: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(KAL_SRC) tests/test_kalcmanu_steering.c -o build/test-kalcmanu-steering-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-kalcmanu-steering-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
kalcmanu-steering-differential:
	python3 tools/interp_routine_differential.py kalcmanu-steering --cases 512 --freeze
	git diff --exit-code -- tests/kalcmanu_steering_vectors.h tests/kalcmanu_steering_c64_vectors.h tools/mk/kalcmanu-steering.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
include tools/mk/kalcmanu-steering.groups
KAL_C64_PRGS = $(foreach g,$(KALCMANU_STEERING_GROUPS),build/kalcmanu-steering-$(g).prg)
KAL_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c tests/kalcmanu_steering_target.c
KAL_C64_lm = src/lm_kalcmanu_steering.c src/lm_interp_orders.c src/lm_interp_steps.c
KAL_C64_cm = src/cm_kalcmanu_steering.c src/cm_interp_orders.c src/cm_interp_steps.c
c64-kalcmanu-steering: $(KAL_C64_PRGS)
build/kalcmanu-steering-%.prg: $(KAL_SRC) platform/c64/kalcmanu_steering_diagnostic.c tools/mk/kalcmanu-steering.groups $(KAL_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(KAL_MOS_FLAGS) $(KALCMANU_STEERING_GROUP_FLAGS_$*) -Wl,-Map,build/kalcmanu-steering-$*.map $(KAL_C64_$(KALCMANU_STEERING_GROUP_MISSION_$*)) $(KAL_C64_COMMON) platform/c64/kalcmanu_steering_diagnostic.c -o $@
vice-kalcmanu-steering: $(KAL_C64_PRGS)
	for p in $(KAL_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
