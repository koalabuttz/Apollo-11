# SPDX-License-Identifier: MIT
# Milestone cm-p76-file (lane A): the Comanche055 P76 (target delta-V) routines compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
CM_P76_FILE_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
CM_P76_FILE_SRC = src/cm_p76_file.c tests/cm_p76_file_target.c $(CM_P76_FILE_COMMON)
CM_P76_FILE_HDR = include/cm_p76_file.h tests/interp_case.h tests/cm_p76_file_vectors.h tests/cm_p76_file_c64_vectors.h
CM_P76_FILE_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -DIRC_CHANNELS
.PHONY: test-cm-p76-file sanitize-cm-p76-file c64-cm-p76-file vice-cm-p76-file cm-p76-file-differential
all: build/test-cm-p76-file
test: test-cm-p76-file
sanitize: sanitize-cm-p76-file
build/test-cm-p76-file: $(CM_P76_FILE_SRC) tests/test_cm_p76_file.c $(CM_P76_FILE_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_P76_FILE_SRC) tests/test_cm_p76_file.c -o $@
test-cm-p76-file: build/test-cm-p76-file
	./build/test-cm-p76-file
sanitize-cm-p76-file: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_P76_FILE_SRC) tests/test_cm_p76_file.c -o build/test-cm-p76-file-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-p76-file-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Comanche055 listing build).
cm-p76-file-differential:
	python3 tools/interp_routine_differential.py cm-p76-file --cases 512 --freeze
	git diff --exit-code -- tests/cm_p76_file_vectors.h tests/cm_p76_file_c64_vectors.h tools/mk/cm-p76-file.groups
# The differential again with the oracle tracing the words it executes
# and reads (tools/oracle_trace), then every instruction of the file checked
# executed (interpretive code: read by the interpreter).
.PHONY: cm-p76-file-closure
cm-p76-file-closure:
	rm -f build/cm-p76-file-executed.txt build/cm-p76-file-read.txt build/interp-routine-reference build/interp-routine-reference-flight-device
	PATH=$(CURDIR)/tools/oracle_trace:$$PATH AGC_EXECUTED_WORDS=$(CURDIR)/build/cm-p76-file-executed.txt AGC_READ_WORDS=$(CURDIR)/build/cm-p76-file-read.txt $(MAKE) --no-print-directory cm-p76-file-differential
	rm -f build/interp-routine-reference build/interp-routine-reference-flight-device
	python3 tools/interp_executed_closure.py cm-p76-file
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/cm-p76-file.groups
CM_P76_FILE_C64_PRGS = $(foreach g,$(CM_P76_FILE_GROUPS),build/cm-p76-file-$(g).prg)
CM_P76_FILE_C64_SRC = src/cm_p76_file.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/cm_p76_file_target.c
c64-cm-p76-file: $(CM_P76_FILE_C64_PRGS)
build/cm-p76-file-%.prg: $(CM_P76_FILE_SRC) platform/c64/cm_p76_file_diagnostic.c tools/mk/cm-p76-file.groups $(CM_P76_FILE_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CM_P76_FILE_MOS_FLAGS) $(CM_P76_FILE_GROUP_FLAGS_$*) -Wl,-Map,build/cm-p76-file-$*.map $(CM_P76_FILE_C64_SRC) platform/c64/cm_p76_file_diagnostic.c -o $@
vice-cm-p76-file: $(CM_P76_FILE_C64_PRGS)
	for p in $(CM_P76_FILE_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
