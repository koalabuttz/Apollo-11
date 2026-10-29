# SPDX-License-Identifier: MIT
# Milestone lm-powered-subs (lane A): the Luminary099 POWERED_FLIGHT_SUBROUTINES routines compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
LM_POWERED_SUBS_COMMON = src/lm_dmpsub.c src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
LM_POWERED_SUBS_SRC = src/lm_powered_subs.c tests/lm_powered_subs_target.c $(LM_POWERED_SUBS_COMMON)
LM_POWERED_SUBS_HDR = include/lm_powered_subs.h tests/interp_case.h tests/lm_powered_subs_vectors.h tests/lm_powered_subs_c64_vectors.h
LM_POWERED_SUBS_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -DIRC_CHANNELS -DAGC_TRANSFER_DANZIG_MARKER
.PHONY: test-lm-powered-subs sanitize-lm-powered-subs c64-lm-powered-subs vice-lm-powered-subs lm-powered-subs-differential
all: build/test-lm-powered-subs
test: test-lm-powered-subs
sanitize: sanitize-lm-powered-subs
build/test-lm-powered-subs: $(LM_POWERED_SUBS_SRC) tests/test_lm_powered_subs.c $(LM_POWERED_SUBS_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_POWERED_SUBS_SRC) tests/test_lm_powered_subs.c -o $@
test-lm-powered-subs: build/test-lm-powered-subs
	./build/test-lm-powered-subs
sanitize-lm-powered-subs: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(LM_POWERED_SUBS_SRC) tests/test_lm_powered_subs.c -o build/test-lm-powered-subs-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-powered-subs-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Luminary099 listing build).
lm-powered-subs-differential:
	python3 tools/interp_routine_differential.py lm-powered-subs --cases 512 --freeze
	git diff --exit-code -- tests/lm_powered_subs_vectors.h tests/lm_powered_subs_c64_vectors.h tools/mk/lm-powered-subs.groups
# The differential again with the oracle tracing the words it executes
# and reads (tools/oracle_trace), then every instruction of the file checked
# executed (interpretive code: read by the interpreter). TSTPOINT's +0 exit
# cannot be reached.
.PHONY: lm-powered-subs-closure
lm-powered-subs-closure:
	rm -f build/lm-powered-subs-executed.txt build/lm-powered-subs-read.txt build/interp-routine-reference build/interp-routine-reference-flight-device
	PATH=$(CURDIR)/tools/oracle_trace:$$PATH AGC_EXECUTED_WORDS=$(CURDIR)/build/lm-powered-subs-executed.txt AGC_READ_WORDS=$(CURDIR)/build/lm-powered-subs-read.txt $(MAKE) --no-print-directory lm-powered-subs-differential
	rm -f build/interp-routine-reference build/interp-routine-reference-flight-device
	python3 tools/interp_executed_closure.py lm-powered-subs --unreached '23,3757=TSTPOINT +0 exit: DIM of +1 gives -0 in the ones-complement adder, and AX*SR*T enters with +-3, so CCS DEXDEX never sees +0'
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/lm-powered-subs.groups
LM_POWERED_SUBS_C64_PRGS = $(foreach g,$(LM_POWERED_SUBS_GROUPS),build/lm-powered-subs-$(g).prg)
LM_POWERED_SUBS_C64_SRC = src/lm_powered_subs.c src/lm_dmpsub.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/lm_powered_subs_target.c
c64-lm-powered-subs: $(LM_POWERED_SUBS_C64_PRGS)
build/lm-powered-subs-%.prg: $(LM_POWERED_SUBS_SRC) platform/c64/lm_powered_subs_diagnostic.c tools/mk/lm-powered-subs.groups $(LM_POWERED_SUBS_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(LM_POWERED_SUBS_MOS_FLAGS) $(LM_POWERED_SUBS_GROUP_FLAGS_$*) -Wl,-Map,build/lm-powered-subs-$*.map $(LM_POWERED_SUBS_C64_SRC) platform/c64/lm_powered_subs_diagnostic.c -o $@
vice-lm-powered-subs: $(LM_POWERED_SUBS_C64_PRGS)
	for p in $(LM_POWERED_SUBS_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
