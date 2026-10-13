# SPDX-License-Identifier: MIT
# Milestone sband-antenna (lane D1): Luminary099 S-BAND_ANTENNA_FOR_LM
# and Comanche055 S-BAND_ANTENNA_FOR_CM compiled by tools/interp_compile.py into
# native routines of the control-transfer ABI.
SBA_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c
SBA_SRC = src/lm_sband_antenna.c src/cm_sband_antenna.c tests/sband_antenna_target.c $(SBA_COMMON)
SBA_HDR = include/lm_sband_antenna.h include/cm_sband_antenna.h tests/interp_case.h tests/sband_antenna_vectors.h tests/sband_antenna_c64_vectors.h
SBA_MOS_FLAGS = -std=c99 -Oz -fno-inline -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -DIRC_INPUTS
.PHONY: test-sband-antenna sanitize-sband-antenna c64-sband-antenna vice-sband-antenna sband-antenna-differential
all: build/test-sband-antenna
test: test-sband-antenna
sanitize: sanitize-sband-antenna
build/test-sband-antenna: $(SBA_SRC) tests/test_sband_antenna.c $(SBA_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SBA_SRC) tests/test_sband_antenna.c -o $@
test-sband-antenna: build/test-sband-antenna
	./build/test-sband-antenna
sanitize-sband-antenna: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(SBA_SRC) tests/test_sband_antenna.c -o build/test-sband-antenna-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-sband-antenna-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
sband-antenna-differential:
	python3 tools/interp_routine_differential.py sband-antenna --cases 512 --freeze
	git diff --exit-code -- tests/sband_antenna_vectors.h tests/sband_antenna_c64_vectors.h tools/mk/sband-antenna.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
include tools/mk/sband-antenna.groups
SBA_C64_PRGS = $(foreach g,$(SBAND_ANTENNA_GROUPS),build/sband-antenna-$(g).prg)
SBA_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c tests/sband_antenna_target.c
SBA_C64_lm = src/lm_sband_antenna.c src/lm_interp_orders.c src/lm_interp_steps.c
SBA_C64_cm = src/cm_sband_antenna.c src/cm_interp_orders.c src/cm_interp_steps.c
c64-sband-antenna: $(SBA_C64_PRGS)
build/sband-antenna-%.prg: $(SBA_SRC) platform/c64/sband_antenna_diagnostic.c tools/mk/sband-antenna.groups $(SBA_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(SBA_MOS_FLAGS) $(SBAND_ANTENNA_GROUP_FLAGS_$*) -Wl,-Map,build/sband-antenna-$*.map $(SBA_C64_$(SBAND_ANTENNA_GROUP_MISSION_$*)) $(SBA_C64_COMMON) platform/c64/sband_antenna_diagnostic.c -o $@
vice-sband-antenna: $(SBA_C64_PRGS)
	for p in $(SBA_C64_PRGS); do echo "PRG $$(basename $$p) sha256 $$(sha256sum $$p | cut -c1-64)"; python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
