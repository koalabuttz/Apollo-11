# SPDX-License-Identifier: MIT
# Milestone cm-powered-subs (lane A): the Comanche055 POWERED_FLIGHT_SUBROUTINES routines compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
CM_POWERED_SUBS_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
CM_POWERED_SUBS_SRC = src/cm_powered_subs.c tests/cm_powered_subs_target.c $(CM_POWERED_SUBS_COMMON)
CM_POWERED_SUBS_HDR = include/cm_powered_subs.h tests/interp_case.h tests/cm_powered_subs_vectors.h tests/cm_powered_subs_c64_vectors.h
CM_POWERED_SUBS_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -DAGC_TRANSFER_DANZIG_MARKER -DIRC_CHANNELS -DAGC_CHANNELS_HW=irc_cm_hw
.PHONY: test-cm-powered-subs sanitize-cm-powered-subs c64-cm-powered-subs vice-cm-powered-subs cm-powered-subs-differential
all: build/test-cm-powered-subs
test: test-cm-powered-subs
sanitize: sanitize-cm-powered-subs
build/test-cm-powered-subs: $(CM_POWERED_SUBS_SRC) tests/test_cm_powered_subs.c $(CM_POWERED_SUBS_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_POWERED_SUBS_SRC) tests/test_cm_powered_subs.c -o $@
test-cm-powered-subs: build/test-cm-powered-subs
	./build/test-cm-powered-subs
sanitize-cm-powered-subs: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_POWERED_SUBS_SRC) tests/test_cm_powered_subs.c -o build/test-cm-powered-subs-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-powered-subs-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Comanche055 listing build).
cm-powered-subs-differential:
	python3 tools/interp_routine_differential.py cm-powered-subs --cases 512 --freeze
	git diff --exit-code -- tests/cm_powered_subs_vectors.h tests/cm_powered_subs_c64_vectors.h tools/mk/cm-powered-subs.groups
# The differential again with the oracle tracing the words it executes
# and reads (tools/oracle_trace), then every instruction of the file checked
# executed (interpretive code: read by the interpreter). TSTPOINT's +0 exit
# cannot be reached.
.PHONY: cm-powered-subs-closure
cm-powered-subs-closure:
	rm -f build/cm-powered-subs-executed.txt build/cm-powered-subs-read.txt build/interp-routine-reference build/interp-routine-reference-flight-device
	PATH=$(CURDIR)/tools/oracle_trace:$$PATH AGC_EXECUTED_WORDS=$(CURDIR)/build/cm-powered-subs-executed.txt AGC_READ_WORDS=$(CURDIR)/build/cm-powered-subs-read.txt $(MAKE) --no-print-directory cm-powered-subs-differential
	rm -f build/interp-routine-reference build/interp-routine-reference-flight-device
	python3 tools/interp_executed_closure.py cm-powered-subs --unreached '23,3732=TSTPOINT +0 exit: DIM of +1 gives -0 in the ones-complement adder, and AX*SR*T enters with +-3, so CCS DEXDEX never sees +0'
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/cm-powered-subs.groups
CM_POWERED_SUBS_C64_PRGS = $(foreach g,$(CM_POWERED_SUBS_GROUPS),build/cm-powered-subs-$(g).prg)
CM_POWERED_SUBS_C64_SRC = src/cm_powered_subs.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/cm_powered_subs_target.c
c64-cm-powered-subs: $(CM_POWERED_SUBS_C64_PRGS)
build/cm-powered-subs-%.prg: $(CM_POWERED_SUBS_SRC) platform/c64/cm_powered_subs_diagnostic.c tools/mk/cm-powered-subs.groups $(CM_POWERED_SUBS_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CM_POWERED_SUBS_MOS_FLAGS) $(CM_POWERED_SUBS_GROUP_FLAGS_$*) -Wl,-Map,build/cm-powered-subs-$*.map $(CM_POWERED_SUBS_C64_SRC) platform/c64/cm_powered_subs_diagnostic.c -o $@
vice-cm-powered-subs: $(CM_POWERED_SUBS_C64_PRGS)
	for p in $(CM_POWERED_SUBS_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
