# SPDX-License-Identifier: MIT
# Milestone lm-p76-file (lane A): the Luminary099 P76 (target delta-V) routines compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
LM_P76_FILE_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
LM_P76_FILE_SRC = src/lm_p76_file.c tests/lm_p76_file_target.c $(LM_P76_FILE_COMMON)
LM_P76_FILE_HDR = include/lm_p76_file.h tests/interp_case.h tests/lm_p76_file_vectors.h tests/lm_p76_file_c64_vectors.h
LM_P76_FILE_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -DIRC_CHANNELS
.PHONY: test-lm-p76-file sanitize-lm-p76-file c64-lm-p76-file vice-lm-p76-file lm-p76-file-differential
all: build/test-lm-p76-file
test: test-lm-p76-file
sanitize: sanitize-lm-p76-file
build/test-lm-p76-file: $(LM_P76_FILE_SRC) tests/test_lm_p76_file.c $(LM_P76_FILE_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_P76_FILE_SRC) tests/test_lm_p76_file.c -o $@
test-lm-p76-file: build/test-lm-p76-file
	./build/test-lm-p76-file
sanitize-lm-p76-file: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(LM_P76_FILE_SRC) tests/test_lm_p76_file.c -o build/test-lm-p76-file-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-p76-file-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Luminary099 listing build).
lm-p76-file-differential:
	python3 tools/interp_routine_differential.py lm-p76-file --cases 512 --freeze
	git diff --exit-code -- tests/lm_p76_file_vectors.h tests/lm_p76_file_c64_vectors.h tools/mk/lm-p76-file.groups
# The differential again with the oracle tracing the words it executes
# and reads (tools/oracle_trace), then every instruction of the file checked
# executed (interpretive code: read by the interpreter).
.PHONY: lm-p76-file-closure
lm-p76-file-closure:
	rm -f build/lm-p76-file-executed.txt build/lm-p76-file-read.txt build/interp-routine-reference build/interp-routine-reference-flight-device
	PATH=$(CURDIR)/tools/oracle_trace:$$PATH AGC_EXECUTED_WORDS=$(CURDIR)/build/lm-p76-file-executed.txt AGC_READ_WORDS=$(CURDIR)/build/lm-p76-file-read.txt $(MAKE) --no-print-directory lm-p76-file-differential
	rm -f build/interp-routine-reference build/interp-routine-reference-flight-device
	python3 tools/interp_executed_closure.py lm-p76-file
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/lm-p76-file.groups
LM_P76_FILE_C64_PRGS = $(foreach g,$(LM_P76_FILE_GROUPS),build/lm-p76-file-$(g).prg)
LM_P76_FILE_C64_SRC = src/lm_p76_file.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/lm_p76_file_target.c
c64-lm-p76-file: $(LM_P76_FILE_C64_PRGS)
build/lm-p76-file-%.prg: $(LM_P76_FILE_SRC) platform/c64/lm_p76_file_diagnostic.c tools/mk/lm-p76-file.groups $(LM_P76_FILE_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(LM_P76_FILE_MOS_FLAGS) $(LM_P76_FILE_GROUP_FLAGS_$*) -Wl,-Map,build/lm-p76-file-$*.map $(LM_P76_FILE_C64_SRC) platform/c64/lm_p76_file_diagnostic.c -o $@
vice-lm-p76-file: $(LM_P76_FILE_C64_PRGS)
	for p in $(LM_P76_FILE_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
