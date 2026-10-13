# SPDX-License-Identifier: MIT
# Milestone cm-stable-orbit (lane G): Comanche055 STABLE_ORBIT compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
CM_STABLE_ORBIT_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
CM_STABLE_ORBIT_SRC = src/cm_stable_orbit.c tests/cm_stable_orbit_target.c $(CM_STABLE_ORBIT_COMMON)
CM_STABLE_ORBIT_HDR = include/cm_stable_orbit.h tests/interp_case.h tests/cm_stable_orbit_vectors.h tests/cm_stable_orbit_c64_vectors.h
CM_STABLE_ORBIT_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -fstatic-stack -mlto-zp=110 -DIRC_CHANNELS
.PHONY: test-cm-stable-orbit sanitize-cm-stable-orbit c64-cm-stable-orbit vice-cm-stable-orbit cm-stable-orbit-differential
all: build/test-cm-stable-orbit
test: test-cm-stable-orbit
sanitize: sanitize-cm-stable-orbit
build/test-cm-stable-orbit: $(CM_STABLE_ORBIT_SRC) tests/test_cm_stable_orbit.c $(CM_STABLE_ORBIT_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_STABLE_ORBIT_SRC) tests/test_cm_stable_orbit.c -o $@
test-cm-stable-orbit: build/test-cm-stable-orbit
	./build/test-cm-stable-orbit
sanitize-cm-stable-orbit: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_STABLE_ORBIT_SRC) tests/test_cm_stable_orbit.c -o build/test-cm-stable-orbit-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-stable-orbit-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Comanche055 listing build).
cm-stable-orbit-differential:
	python3 tools/interp_routine_differential.py cm-stable-orbit --cases 256 --freeze
	git diff --exit-code -- tests/cm_stable_orbit_vectors.h tests/cm_stable_orbit_c64_vectors.h tools/mk/cm-stable-orbit.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/cm-stable-orbit.groups
CM_STABLE_ORBIT_C64_PRGS = $(foreach g,$(CM_STABLE_ORBIT_GROUPS),build/cm-stable-orbit-$(g).prg)
CM_STABLE_ORBIT_C64_SRC = src/cm_stable_orbit.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/cm_stable_orbit_target.c
c64-cm-stable-orbit: $(CM_STABLE_ORBIT_C64_PRGS)
build/cm-stable-orbit-%.prg: $(CM_STABLE_ORBIT_SRC) platform/c64/cm_stable_orbit_diagnostic.c tools/mk/cm-stable-orbit.groups $(CM_STABLE_ORBIT_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CM_STABLE_ORBIT_MOS_FLAGS) $(CM_STABLE_ORBIT_GROUP_FLAGS_$*) -Wl,-Map,build/cm-stable-orbit-$*.map $(CM_STABLE_ORBIT_C64_SRC) platform/c64/cm_stable_orbit_diagnostic.c -o $@
vice-cm-stable-orbit: $(CM_STABLE_ORBIT_C64_PRGS)
	for p in $(CM_STABLE_ORBIT_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
