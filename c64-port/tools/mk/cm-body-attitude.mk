# SPDX-License-Identifier: MIT
# Milestone cm-body-attitude (lane G): Comanche055 CM_BODY_ATTITUDE compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
CM_BODY_ATTITUDE_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c
CM_BODY_ATTITUDE_SRC = src/cm_body_attitude.c tests/cm_body_attitude_target.c $(CM_BODY_ATTITUDE_COMMON)
CM_BODY_ATTITUDE_HDR = include/cm_body_attitude.h tests/interp_case.h tests/cm_body_attitude_vectors.h tests/cm_body_attitude_c64_vectors.h
CM_BODY_ATTITUDE_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline
.PHONY: test-cm-body-attitude sanitize-cm-body-attitude c64-cm-body-attitude vice-cm-body-attitude cm-body-attitude-differential
all: build/test-cm-body-attitude
test: test-cm-body-attitude
sanitize: sanitize-cm-body-attitude
build/test-cm-body-attitude: $(CM_BODY_ATTITUDE_SRC) tests/test_cm_body_attitude.c $(CM_BODY_ATTITUDE_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_BODY_ATTITUDE_SRC) tests/test_cm_body_attitude.c -o $@
test-cm-body-attitude: build/test-cm-body-attitude
	./build/test-cm-body-attitude
sanitize-cm-body-attitude: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_BODY_ATTITUDE_SRC) tests/test_cm_body_attitude.c -o build/test-cm-body-attitude-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-body-attitude-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Comanche055 listing build).
cm-body-attitude-differential:
	python3 tools/interp_routine_differential.py cm-body-attitude --cases 256 --freeze
	git diff --exit-code -- tests/cm_body_attitude_vectors.h tests/cm_body_attitude_c64_vectors.h tools/mk/cm-body-attitude.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/cm-body-attitude.groups
CM_BODY_ATTITUDE_C64_PRGS = $(foreach g,$(CM_BODY_ATTITUDE_GROUPS),build/cm-body-attitude-$(g).prg)
CM_BODY_ATTITUDE_C64_SRC = src/cm_body_attitude.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c tests/cm_body_attitude_target.c
c64-cm-body-attitude: $(CM_BODY_ATTITUDE_C64_PRGS)
build/cm-body-attitude-%.prg: $(CM_BODY_ATTITUDE_SRC) platform/c64/cm_body_attitude_diagnostic.c tools/mk/cm-body-attitude.groups $(CM_BODY_ATTITUDE_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CM_BODY_ATTITUDE_MOS_FLAGS) $(CM_BODY_ATTITUDE_GROUP_FLAGS_$*) -Wl,-Map,build/cm-body-attitude-$*.map $(CM_BODY_ATTITUDE_C64_SRC) platform/c64/cm_body_attitude_diagnostic.c -o $@
vice-cm-body-attitude: $(CM_BODY_ATTITUDE_C64_PRGS)
	for p in $(CM_BODY_ATTITUDE_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
