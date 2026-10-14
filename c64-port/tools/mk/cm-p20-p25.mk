# SPDX-License-Identifier: MIT
# Milestone cm-p20-p25 (lane D1b): Comanche055 P20-P25 compiled by
# tools/interp_compile.py into
# native routines of the control-transfer ABI.
CP20_COMMON = src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c src/rtb_modes.c tests/interp_case.c
CP20_SRC = src/cm_p20_p25.c tests/cm_p20_p25_target.c $(CP20_COMMON)
CP20_HDR = include/cm_p20_p25.h tests/interp_case.h tests/cm_p20_p25_vectors.h tests/cm_p20_p25_c64_vectors.h
CP20_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror
.PHONY: test-cm-p20-p25 sanitize-cm-p20-p25 c64-cm-p20-p25 vice-cm-p20-p25 cm-p20-p25-differential
all: build/test-cm-p20-p25
test: test-cm-p20-p25
sanitize: sanitize-cm-p20-p25
build/test-cm-p20-p25: $(CP20_SRC) tests/test_cm_p20_p25.c $(CP20_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CP20_SRC) tests/test_cm_p20_p25.c -o $@
test-cm-p20-p25: build/test-cm-p20-p25
	./build/test-cm-p20-p25
sanitize-cm-p20-p25: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CP20_SRC) tests/test_cm_p20_p25.c -o build/test-cm-p20-p25-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-p20-p25-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
cm-p20-p25-differential:
	python3 tools/interp_routine_differential.py cm-p20-p25 --cases 256 --freeze
	git diff --exit-code -- tests/cm_p20_p25_vectors.h tests/cm_p20_p25_c64_vectors.h tools/mk/cm-p20-p25.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
include tools/mk/cm-p20-p25.groups
CP20_C64_PRGS = $(foreach g,$(CM_P20_P25_GROUPS),build/cm-p20-p25-$(g).prg)
CP20_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c src/rtb_modes.c tests/interp_case.c tests/cm_p20_p25_target.c
CP20_C64_cm = src/cm_p20_p25.c src/cm_interp_orders.c src/cm_interp_steps.c
c64-cm-p20-p25: $(CP20_C64_PRGS)
build/cm-p20-p25-%.prg: $(CP20_SRC) platform/c64/cm_p20_p25_diagnostic.c tools/mk/cm-p20-p25.groups $(CP20_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CP20_MOS_FLAGS) $(CM_P20_P25_GROUP_FLAGS_$*) -Wl,-Map,build/cm-p20-p25-$*.map $(CP20_C64_$(CM_P20_P25_GROUP_MISSION_$*)) $(CP20_C64_COMMON) platform/c64/cm_p20_p25_diagnostic.c -o $@
vice-cm-p20-p25: $(CP20_C64_PRGS)
	for p in $(CP20_C64_PRGS); do echo "PRG $$(basename $$p) sha256 $$(sha256sum $$p | cut -c1-64)"; python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
