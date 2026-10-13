# SPDX-License-Identifier: MIT
# Milestone findcduw (lane A2): Luminary099 FINDCDUW--GUIDAP_INTERFACE
# compiled by tools/interp_compile.py into native routines of the
# control-transfer ABI (RTB NORMUNIT and QUICTRIG through
# src/lm_findcduw_rtb.c; PAUTNO's RAND CHAN30/CHAN31 from the latched
# input channels, include/agc_input_channels.h).
FCW_NATIVE = src/lm_findcduw_rtb.c src/rtb_vector.c src/rtb_unit.c src/guidance_unit_phased.c src/guidance_length_phased.c src/guidance_norm.c src/lm_powered_quick.c src/lm_single_trig.c src/rtb_angles.c
FCW_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
FCW_SRC = src/lm_findcduw.c tests/findcduw_target.c $(FCW_NATIVE) $(FCW_COMMON)
FCW_HDR = include/lm_findcduw.h include/lm_findcduw_rtb.h include/agc_input_channels.h tests/interp_case.h tests/findcduw_vectors.h tests/findcduw_c64_vectors.h
# (the spec's c64_flags; -DIRC_INPUTS for the latched input channels)
FCW_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -fstatic-stack -mlto-zp=110 -DIRC_INPUTS
.PHONY: test-findcduw sanitize-findcduw c64-findcduw vice-findcduw findcduw-differential
all: build/test-findcduw
test: test-findcduw
sanitize: sanitize-findcduw
build/test-findcduw: $(FCW_SRC) tests/test_findcduw.c $(FCW_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(FCW_SRC) tests/test_findcduw.c -o $@
test-findcduw: build/test-findcduw
	./build/test-findcduw
sanitize-findcduw: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(FCW_SRC) tests/test_findcduw.c -o build/test-findcduw-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-findcduw-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the listing build).
findcduw-differential:
	python3 tools/interp_routine_differential.py findcduw --cases 256 --freeze
	git diff --exit-code -- tests/findcduw_vectors.h tests/findcduw_c64_vectors.h tools/mk/findcduw.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox, the image under KERNAL).
-include tools/mk/findcduw.groups
FCW_C64_PRGS = $(foreach g,$(FINDCDUW_GROUPS),build/findcduw-$(g).prg)
FCW_C64 = src/lm_findcduw.c src/lm_interp_orders.c src/lm_interp_steps.c $(FCW_NATIVE) src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/findcduw_target.c
c64-findcduw: $(FCW_C64_PRGS)
build/findcduw-%.prg: $(FCW_SRC) platform/c64/findcduw_diagnostic.c tools/mk/findcduw.groups $(FCW_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(FCW_MOS_FLAGS) $(FINDCDUW_GROUP_FLAGS_$*) -Wl,-Map,build/findcduw-$*.map $(FCW_C64) platform/c64/findcduw_diagnostic.c -o $@
vice-findcduw: $(FCW_C64_PRGS)
	for p in $(FCW_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
