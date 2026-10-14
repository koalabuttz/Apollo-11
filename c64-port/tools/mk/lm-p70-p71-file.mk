# SPDX-License-Identifier: MIT
# Milestone lm-p70-p71-file (lane A): the Luminary099 P70-P71 (DPS and APS abort) routines compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI (channel orders through the program's
# agc_hardware).
LM_P70_P71_FILE_COMMON = src/rtb_angles.c src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
LM_P70_P71_FILE_SRC = src/lm_p70_p71_file.c tests/lm_p70_p71_file_target.c $(LM_P70_P71_FILE_COMMON)
LM_P70_P71_FILE_HDR = include/lm_p70_p71_file.h tests/interp_case.h tests/lm_p70_p71_file_vectors.h tests/lm_p70_p71_file_c64_vectors.h
LM_P70_P71_FILE_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -fstatic-stack -mlto-zp=110 -DIRC_CHANNELS -DIRC_INPUTS
.PHONY: test-lm-p70-p71-file sanitize-lm-p70-p71-file c64-lm-p70-p71-file vice-lm-p70-p71-file lm-p70-p71-file-differential
all: build/test-lm-p70-p71-file
test: test-lm-p70-p71-file
sanitize: sanitize-lm-p70-p71-file
build/test-lm-p70-p71-file: $(LM_P70_P71_FILE_SRC) tests/test_lm_p70_p71_file.c $(LM_P70_P71_FILE_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_P70_P71_FILE_SRC) tests/test_lm_p70_p71_file.c -o $@
test-lm-p70-p71-file: build/test-lm-p70-p71-file
	./build/test-lm-p70-p71-file
sanitize-lm-p70-p71-file: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(LM_P70_P71_FILE_SRC) tests/test_lm_p70_p71_file.c -o build/test-lm-p70-p71-file-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-p70-p71-file-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Luminary099 listing build).
lm-p70-p71-file-differential:
	python3 tools/interp_routine_differential.py lm-p70-p71-file --cases 512 --freeze
	git diff --exit-code -- tests/lm_p70_p71_file_vectors.h tests/lm_p70_p71_file_c64_vectors.h tools/mk/lm-p70-p71-file.groups
# The differential again with the oracle tracing the words it executes
# and reads (tools/oracle_trace), then every instruction of the file checked
# executed (interpretive code: read by the interpreter).
.PHONY: lm-p70-p71-file-closure
lm-p70-p71-file-closure:
	rm -f build/lm-p70-p71-file-executed.txt build/lm-p70-p71-file-read.txt build/interp-routine-reference build/interp-routine-reference-flight-device
	PATH=$(CURDIR)/tools/oracle_trace:$$PATH AGC_EXECUTED_WORDS=$(CURDIR)/build/lm-p70-p71-file-executed.txt AGC_READ_WORDS=$(CURDIR)/build/lm-p70-p71-file-read.txt $(MAKE) --no-print-directory lm-p70-p71-file-differential
	rm -f build/interp-routine-reference build/interp-routine-reference-flight-device
	python3 tools/interp_executed_closure.py lm-p70-p71-file
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/lm-p70-p71-file.groups
LM_P70_P71_FILE_C64_PRGS = $(foreach g,$(LM_P70_P71_FILE_GROUPS),build/lm-p70-p71-file-$(g).prg)
LM_P70_P71_FILE_C64_SRC = src/lm_p70_p71_file.c src/rtb_angles.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/interp_cut.c tests/lm_p70_p71_file_target.c
c64-lm-p70-p71-file: $(LM_P70_P71_FILE_C64_PRGS)
build/lm-p70-p71-file-%.prg: $(LM_P70_P71_FILE_SRC) platform/c64/lm_p70_p71_file_diagnostic.c tools/mk/lm-p70-p71-file.groups $(LM_P70_P71_FILE_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(LM_P70_P71_FILE_MOS_FLAGS) $(LM_P70_P71_FILE_GROUP_FLAGS_$*) -Wl,-Map,build/lm-p70-p71-file-$*.map $(LM_P70_P71_FILE_C64_SRC) platform/c64/lm_p70_p71_file_diagnostic.c -o $@
vice-lm-p70-p71-file: $(LM_P70_P71_FILE_C64_PRGS)
	for p in $(LM_P70_P71_FILE_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
