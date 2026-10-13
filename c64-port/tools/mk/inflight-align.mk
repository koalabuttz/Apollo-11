# SPDX-License-Identifier: MIT
# Milestone inflight-align (lane D1b): Luminary099 and Comanche055
# INFLIGHT_ALIGNMENT_ROUTINES compiled by tools/interp_compile.py into
# native routines of the control-transfer ABI.
IFA_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c
IFA_SRC = src/lm_inflight_align.c src/cm_inflight_align.c tests/inflight_align_target.c $(IFA_COMMON)
IFA_HDR = include/lm_inflight_align.h include/cm_inflight_align.h tests/interp_case.h tests/inflight_align_vectors.h tests/inflight_align_c64_vectors.h
IFA_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror
.PHONY: test-inflight-align sanitize-inflight-align c64-inflight-align vice-inflight-align inflight-align-differential
all: build/test-inflight-align
test: test-inflight-align
sanitize: sanitize-inflight-align
build/test-inflight-align: $(IFA_SRC) tests/test_inflight_align.c $(IFA_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(IFA_SRC) tests/test_inflight_align.c -o $@
test-inflight-align: build/test-inflight-align
	./build/test-inflight-align
sanitize-inflight-align: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(IFA_SRC) tests/test_inflight_align.c -o build/test-inflight-align-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-inflight-align-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
inflight-align-differential:
	python3 tools/interp_routine_differential.py inflight-align --cases 512 --freeze
	git diff --exit-code -- tests/inflight_align_vectors.h tests/inflight_align_c64_vectors.h tools/mk/inflight-align.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
include tools/mk/inflight-align.groups
IFA_C64_PRGS = $(foreach g,$(INFLIGHT_ALIGN_GROUPS),build/inflight-align-$(g).prg)
IFA_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c tests/inflight_align_target.c
IFA_C64_lm = src/lm_inflight_align.c src/lm_interp_orders.c src/lm_interp_steps.c
IFA_C64_cm = src/cm_inflight_align.c src/cm_interp_orders.c src/cm_interp_steps.c
c64-inflight-align: $(IFA_C64_PRGS)
build/inflight-align-%.prg: $(IFA_SRC) platform/c64/inflight_align_diagnostic.c tools/mk/inflight-align.groups $(IFA_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(IFA_MOS_FLAGS) $(INFLIGHT_ALIGN_GROUP_FLAGS_$*) -Wl,-Map,build/inflight-align-$*.map $(IFA_C64_$(INFLIGHT_ALIGN_GROUP_MISSION_$*)) $(IFA_C64_COMMON) platform/c64/inflight_align_diagnostic.c -o $@
vice-inflight-align: $(IFA_C64_PRGS)
	for p in $(IFA_C64_PRGS); do echo "PRG $$(basename $$p) sha256 $$(sha256sum $$p | cut -c1-64)"; python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
