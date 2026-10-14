# SPDX-License-Identifier: MIT
# Milestone lm-stable-orbit (lane B): Luminary099 STABLE_ORBIT (P38-P39, P78-P79) compiled by
# tools/interp_compile.py into native routines of the control-transfer ABI.
LSO_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c
LSO_SRC = src/lm_stable_orbit.c tests/lm_stable_orbit_target.c $(LSO_COMMON)
LSO_HDR = include/lm_stable_orbit.h tests/interp_case.h tests/lm_stable_orbit_vectors.h tests/lm_stable_orbit_c64_vectors.h
LSO_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -fstatic-stack -mlto-zp=110
.PHONY: test-lm-stable-orbit sanitize-lm-stable-orbit c64-lm-stable-orbit vice-lm-stable-orbit lm-stable-orbit-differential
all: build/test-lm-stable-orbit
test: test-lm-stable-orbit
sanitize: sanitize-lm-stable-orbit
build/test-lm-stable-orbit: $(LSO_SRC) tests/test_lm_stable_orbit.c $(LSO_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LSO_SRC) tests/test_lm_stable_orbit.c -o $@
test-lm-stable-orbit: build/test-lm-stable-orbit
	./build/test-lm-stable-orbit
sanitize-lm-stable-orbit: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(LSO_SRC) tests/test_lm_stable_orbit.c -o build/test-lm-stable-orbit-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-stable-orbit-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
lm-stable-orbit-differential:
	python3 tools/interp_routine_differential.py lm-stable-orbit --cases 256 --freeze
	git diff --exit-code -- tests/lm_stable_orbit_vectors.h tests/lm_stable_orbit_c64_vectors.h tools/mk/lm-stable-orbit.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
include tools/mk/lm-stable-orbit.groups
LSO_C64_PRGS = $(foreach g,$(LM_STABLE_ORBIT_GROUPS),build/lm-stable-orbit-$(g).prg)
LSO_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c tests/lm_stable_orbit_target.c
LSO_C64_lm = src/lm_stable_orbit.c src/lm_interp_orders.c src/lm_interp_steps.c
c64-lm-stable-orbit: $(LSO_C64_PRGS)
build/lm-stable-orbit-%.prg: $(LSO_SRC) platform/c64/lm_stable_orbit_diagnostic.c tools/mk/lm-stable-orbit.groups $(LSO_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(LSO_MOS_FLAGS) $(LM_STABLE_ORBIT_GROUP_FLAGS_$*) -Wl,-Map,build/lm-stable-orbit-$*.map $(LSO_C64_$(LM_STABLE_ORBIT_GROUP_MISSION_$*)) $(LSO_C64_COMMON) platform/c64/lm_stable_orbit_diagnostic.c -o $@
vice-lm-stable-orbit: $(LSO_C64_PRGS)
	for p in $(LSO_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
