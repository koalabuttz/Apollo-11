# SPDX-License-Identifier: MIT
# Milestone cm-sxtmark (lane D1): Comanche055 SXTMARK compiled by
# tools/interp_compile.py into native routines of the control-transfer ABI.
SXT_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
SXT_SRC = src/cm_sxtmark.c tests/cm_sxtmark_target.c $(SXT_COMMON)
SXT_HDR = include/cm_sxtmark.h tests/interp_case.h tests/cm_sxtmark_vectors.h tests/cm_sxtmark_c64_vectors.h
SXT_MOS_FLAGS = -std=c99 -Oz -fno-inline -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -DIRC_INPUTS -DIRC_CHANNELS -DAGC_CHANNELS_HW=irc_cm_hw
.PHONY: test-cm-sxtmark sanitize-cm-sxtmark c64-cm-sxtmark vice-cm-sxtmark cm-sxtmark-differential
all: build/test-cm-sxtmark
test: test-cm-sxtmark
sanitize: sanitize-cm-sxtmark
build/test-cm-sxtmark: $(SXT_SRC) tests/test_cm_sxtmark.c $(SXT_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SXT_SRC) tests/test_cm_sxtmark.c -o $@
test-cm-sxtmark: build/test-cm-sxtmark
	./build/test-cm-sxtmark
sanitize-cm-sxtmark: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(SXT_SRC) tests/test_cm_sxtmark.c -o build/test-cm-sxtmark-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-sxtmark-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
cm-sxtmark-differential:
	python3 tools/interp_routine_differential.py cm-sxtmark --cases 512 --freeze
	git diff --exit-code -- tests/cm_sxtmark_vectors.h tests/cm_sxtmark_c64_vectors.h tools/mk/cm-sxtmark.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
include tools/mk/cm-sxtmark.groups
SXT_C64_PRGS = $(foreach g,$(CM_SXTMARK_GROUPS),build/cm-sxtmark-$(g).prg)
SXT_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/cm_sxtmark_target.c
SXT_C64_cm = src/cm_sxtmark.c src/cm_interp_orders.c src/cm_interp_steps.c
c64-cm-sxtmark: $(SXT_C64_PRGS)
build/cm-sxtmark-%.prg: $(SXT_SRC) platform/c64/cm_sxtmark_diagnostic.c tools/mk/cm-sxtmark.groups $(SXT_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(SXT_MOS_FLAGS) $(CM_SXTMARK_GROUP_FLAGS_$*) -Wl,-Map,build/cm-sxtmark-$*.map $(SXT_C64_$(CM_SXTMARK_GROUP_MISSION_$*)) $(SXT_C64_COMMON) platform/c64/cm_sxtmark_diagnostic.c -o $@
vice-cm-sxtmark: $(SXT_C64_PRGS)
	for p in $(SXT_C64_PRGS); do echo "PRG $$(basename $$p) sha256 $$(sha256sum $$p | cut -c1-64)"; python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
