# SPDX-License-Identifier: MIT
# Milestone rtb-op-codes (lane D1b, issue #57): Luminary099 and Comanche055
# RTB_OP_CODES (RT8_OP_CODES in the Comanche tree) compiled by tools/interp_compile.py into
# native routines of the control-transfer ABI.
RTB_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
RTB_SRC = src/lm_rtb_op_codes.c src/cm_rtb_op_codes.c tests/rtb_op_codes_target.c $(RTB_COMMON)
RTB_HDR = include/lm_rtb_op_codes.h include/cm_rtb_op_codes.h tests/interp_case.h tests/rtb_op_codes_vectors.h tests/rtb_op_codes_c64_vectors.h
RTB_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror
.PHONY: test-rtb-op-codes sanitize-rtb-op-codes c64-rtb-op-codes vice-rtb-op-codes rtb-op-codes-differential
all: build/test-rtb-op-codes
test: test-rtb-op-codes
sanitize: sanitize-rtb-op-codes
build/test-rtb-op-codes: $(RTB_SRC) tests/test_rtb_op_codes.c $(RTB_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(RTB_SRC) tests/test_rtb_op_codes.c -o $@
test-rtb-op-codes: build/test-rtb-op-codes
	./build/test-rtb-op-codes
sanitize-rtb-op-codes: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(RTB_SRC) tests/test_rtb_op_codes.c -o build/test-rtb-op-codes-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-rtb-op-codes-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
rtb-op-codes-differential:
	python3 tools/interp_routine_differential.py rtb-op-codes --cases 512 --freeze
	git diff --exit-code -- tests/rtb_op_codes_vectors.h tests/rtb_op_codes_c64_vectors.h tools/mk/rtb-op-codes.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
include tools/mk/rtb-op-codes.groups
RTB_C64_PRGS = $(foreach g,$(RTB_OP_CODES_GROUPS),build/rtb-op-codes-$(g).prg)
RTB_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/rtb_op_codes_target.c
RTB_C64_lm = src/lm_rtb_op_codes.c src/lm_interp_orders.c src/lm_interp_steps.c
RTB_C64_cm = src/cm_rtb_op_codes.c src/cm_interp_orders.c src/cm_interp_steps.c
c64-rtb-op-codes: $(RTB_C64_PRGS)
build/rtb-op-codes-%.prg: $(RTB_SRC) platform/c64/rtb_op_codes_diagnostic.c tools/mk/rtb-op-codes.groups $(RTB_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(RTB_MOS_FLAGS) $(RTB_OP_CODES_GROUP_FLAGS_$*) -Wl,-Map,build/rtb-op-codes-$*.map $(RTB_C64_$(RTB_OP_CODES_GROUP_MISSION_$*)) $(RTB_C64_COMMON) platform/c64/rtb_op_codes_diagnostic.c -o $@
vice-rtb-op-codes: $(RTB_C64_PRGS)
	for p in $(RTB_C64_PRGS); do echo "PRG $$(basename $$p) sha256 $$(sha256sum $$p | cut -c1-64)"; python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
