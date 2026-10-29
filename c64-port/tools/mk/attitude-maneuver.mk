# SPDX-License-Identifier: MIT
# Milestone attitude-maneuver (lane D1b): Luminary099 ATTITUDE_MANEUVER_ROUTINE
# and Comanche055 ANGLFIND compiled by tools/interp_compile.py into
# native routines of the control-transfer ABI.
ATT_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c
ATT_SRC = src/lm_attitude_maneuver.c src/cm_attitude_maneuver.c tests/attitude_maneuver_target.c $(ATT_COMMON)
ATT_HDR = include/lm_attitude_maneuver.h include/cm_attitude_maneuver.h tests/interp_case.h tests/attitude_maneuver_vectors.h tests/attitude_maneuver_c64_vectors.h
ATT_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline
.PHONY: test-attitude-maneuver sanitize-attitude-maneuver c64-attitude-maneuver vice-attitude-maneuver attitude-maneuver-differential
all: build/test-attitude-maneuver
test: test-attitude-maneuver
sanitize: sanitize-attitude-maneuver
build/test-attitude-maneuver: $(ATT_SRC) tests/test_attitude_maneuver.c $(ATT_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(ATT_SRC) tests/test_attitude_maneuver.c -o $@
test-attitude-maneuver: build/test-attitude-maneuver
	./build/test-attitude-maneuver
sanitize-attitude-maneuver: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(ATT_SRC) tests/test_attitude_maneuver.c -o build/test-attitude-maneuver-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-attitude-maneuver-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
attitude-maneuver-differential:
	python3 tools/interp_routine_differential.py attitude-maneuver --cases 512 --freeze
	git diff --exit-code -- tests/attitude_maneuver_vectors.h tests/attitude_maneuver_c64_vectors.h tools/mk/attitude-maneuver.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
include tools/mk/attitude-maneuver.groups
ATT_C64_PRGS = $(foreach g,$(ATTITUDE_MANEUVER_GROUPS),build/attitude-maneuver-$(g).prg)
ATT_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c tests/attitude_maneuver_target.c
ATT_C64_lm = src/lm_attitude_maneuver.c src/lm_interp_orders.c src/lm_interp_steps.c
ATT_C64_cm = src/cm_attitude_maneuver.c src/cm_interp_orders.c src/cm_interp_steps.c
c64-attitude-maneuver: $(ATT_C64_PRGS)
build/attitude-maneuver-%.prg: $(ATT_SRC) platform/c64/attitude_maneuver_diagnostic.c tools/mk/attitude-maneuver.groups $(ATT_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(ATT_MOS_FLAGS) $(ATTITUDE_MANEUVER_GROUP_FLAGS_$*) -Wl,-Map,build/attitude-maneuver-$*.map $(ATT_C64_$(ATTITUDE_MANEUVER_GROUP_MISSION_$*)) $(ATT_C64_COMMON) platform/c64/attitude_maneuver_diagnostic.c -o $@
vice-attitude-maneuver: $(ATT_C64_PRGS)
	for p in $(ATT_C64_PRGS); do echo "PRG $$(basename $$p) sha256 $$(sha256sum $$p | cut -c1-64)"; python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
