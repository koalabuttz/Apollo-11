# SPDX-License-Identifier: MIT
# Milestone lm-lambert-aim (lane A): the Luminary099 GENERAL_LAMBERT_AIMPOINT_GUIDANCE (P31) routines compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
LM_LAMBERT_AIM_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
LM_LAMBERT_AIM_SRC = src/lm_lambert_aim.c tests/lm_lambert_aim_target.c $(LM_LAMBERT_AIM_COMMON)
LM_LAMBERT_AIM_HDR = include/lm_lambert_aim.h tests/interp_case.h tests/lm_lambert_aim_vectors.h tests/lm_lambert_aim_c64_vectors.h
LM_LAMBERT_AIM_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -DIRC_CHANNELS
.PHONY: test-lm-lambert-aim sanitize-lm-lambert-aim c64-lm-lambert-aim vice-lm-lambert-aim lm-lambert-aim-differential
all: build/test-lm-lambert-aim
test: test-lm-lambert-aim
sanitize: sanitize-lm-lambert-aim
build/test-lm-lambert-aim: $(LM_LAMBERT_AIM_SRC) tests/test_lm_lambert_aim.c $(LM_LAMBERT_AIM_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_LAMBERT_AIM_SRC) tests/test_lm_lambert_aim.c -o $@
test-lm-lambert-aim: build/test-lm-lambert-aim
	./build/test-lm-lambert-aim
sanitize-lm-lambert-aim: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(LM_LAMBERT_AIM_SRC) tests/test_lm_lambert_aim.c -o build/test-lm-lambert-aim-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-lambert-aim-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Luminary099 listing build).
lm-lambert-aim-differential:
	python3 tools/interp_routine_differential.py lm-lambert-aim --cases 512 --freeze
	git diff --exit-code -- tests/lm_lambert_aim_vectors.h tests/lm_lambert_aim_c64_vectors.h tools/mk/lm-lambert-aim.groups
# The differential again with the oracle tracing the words it executes
# and reads (tools/oracle_trace), then every instruction of the file checked
# executed (interpretive code: read by the interpreter).
.PHONY: lm-lambert-aim-closure
lm-lambert-aim-closure:
	rm -f build/lm-lambert-aim-executed.txt build/lm-lambert-aim-read.txt build/interp-routine-reference build/interp-routine-reference-flight-device
	PATH=$(CURDIR)/tools/oracle_trace:$$PATH AGC_EXECUTED_WORDS=$(CURDIR)/build/lm-lambert-aim-executed.txt AGC_READ_WORDS=$(CURDIR)/build/lm-lambert-aim-read.txt $(MAKE) --no-print-directory lm-lambert-aim-differential
	rm -f build/interp-routine-reference build/interp-routine-reference-flight-device
	python3 tools/interp_executed_closure.py lm-lambert-aim
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/lm-lambert-aim.groups
LM_LAMBERT_AIM_C64_PRGS = $(foreach g,$(LM_LAMBERT_AIM_GROUPS),build/lm-lambert-aim-$(g).prg)
LM_LAMBERT_AIM_C64_SRC = src/lm_lambert_aim.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/lm_lambert_aim_target.c
c64-lm-lambert-aim: $(LM_LAMBERT_AIM_C64_PRGS)
build/lm-lambert-aim-%.prg: $(LM_LAMBERT_AIM_SRC) platform/c64/lm_lambert_aim_diagnostic.c tools/mk/lm-lambert-aim.groups $(LM_LAMBERT_AIM_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(LM_LAMBERT_AIM_MOS_FLAGS) $(LM_LAMBERT_AIM_GROUP_FLAGS_$*) -Wl,-Map,build/lm-lambert-aim-$*.map $(LM_LAMBERT_AIM_C64_SRC) platform/c64/lm_lambert_aim_diagnostic.c -o $@
vice-lm-lambert-aim: $(LM_LAMBERT_AIM_C64_PRGS)
	for p in $(LM_LAMBERT_AIM_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
