# SPDX-License-Identifier: MIT
# Milestone p51-p53 (lane D1b): Luminary099 and Comanche055
# P51-P53 (both missions) compiled by tools/interp_compile.py into
# native routines of the control-transfer ABI.
P51_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c src/rtb_agreement.c src/p51_p53_rtb.c tests/interp_case.c
P51_SRC = src/lm_p51_p53.c src/cm_p51_p53.c tests/p51_p53_target.c $(P51_COMMON)
P51_HDR = include/lm_p51_p53.h include/cm_p51_p53.h tests/interp_case.h tests/p51_p53_vectors.h tests/p51_p53_c64_vectors.h
P51_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror
.PHONY: test-p51-p53 sanitize-p51-p53 c64-p51-p53 vice-p51-p53 p51-p53-differential
all: build/test-p51-p53
test: test-p51-p53
sanitize: sanitize-p51-p53
build/test-p51-p53: $(P51_SRC) tests/test_p51_p53.c $(P51_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(P51_SRC) tests/test_p51_p53.c -o $@
test-p51-p53: build/test-p51-p53
	./build/test-p51-p53
sanitize-p51-p53: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(P51_SRC) tests/test_p51_p53.c -o build/test-p51-p53-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-p51-p53-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
p51-p53-differential:
	python3 tools/interp_routine_differential.py p51-p53 --cases 512 --freeze
	git diff --exit-code -- tests/p51_p53_vectors.h tests/p51_p53_c64_vectors.h tools/mk/p51-p53.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
include tools/mk/p51-p53.groups
P51_C64_PRGS = $(foreach g,$(P51_P53_GROUPS),build/p51-p53-$(g).prg)
P51_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c src/rtb_agreement.c src/p51_p53_rtb.c tests/interp_case.c tests/p51_p53_target.c
P51_C64_lm = src/lm_p51_p53.c src/lm_interp_orders.c src/lm_interp_steps.c
P51_C64_cm = src/cm_p51_p53.c src/cm_interp_orders.c src/cm_interp_steps.c
c64-p51-p53: $(P51_C64_PRGS)
build/p51-p53-%.prg: $(P51_SRC) platform/c64/p51_p53_diagnostic.c tools/mk/p51-p53.groups $(P51_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(P51_MOS_FLAGS) $(P51_P53_GROUP_FLAGS_$*) -Wl,-Map,build/p51-p53-$*.map $(P51_C64_$(P51_P53_GROUP_MISSION_$*)) $(P51_C64_COMMON) platform/c64/p51_p53_diagnostic.c -o $@
vice-p51-p53: $(P51_C64_PRGS)
	for p in $(P51_C64_PRGS); do echo "PRG $$(basename $$p) sha256 $$(sha256sum $$p | cut -c1-64)"; python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
