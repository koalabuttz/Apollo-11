# SPDX-License-Identifier: MIT
# Milestone cm-extended-verbs-file (lane A): the Comanche055 EXTENDED_VERBS (the extended verbs) routines compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
CM_EXTENDED_VERBS_FILE_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
CM_EXTENDED_VERBS_FILE_SRC = src/cm_extended_verbs_file.c tests/cm_extended_verbs_file_target.c $(CM_EXTENDED_VERBS_FILE_COMMON)
CM_EXTENDED_VERBS_FILE_HDR = include/cm_extended_verbs_file.h tests/interp_case.h tests/cm_extended_verbs_file_vectors.h tests/cm_extended_verbs_file_c64_vectors.h
CM_EXTENDED_VERBS_FILE_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -DIRC_CHANNELS
.PHONY: test-cm-extended-verbs-file sanitize-cm-extended-verbs-file c64-cm-extended-verbs-file vice-cm-extended-verbs-file cm-extended-verbs-file-differential
all: build/test-cm-extended-verbs-file
test: test-cm-extended-verbs-file
sanitize: sanitize-cm-extended-verbs-file
build/test-cm-extended-verbs-file: $(CM_EXTENDED_VERBS_FILE_SRC) tests/test_cm_extended_verbs_file.c $(CM_EXTENDED_VERBS_FILE_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_EXTENDED_VERBS_FILE_SRC) tests/test_cm_extended_verbs_file.c -o $@
test-cm-extended-verbs-file: build/test-cm-extended-verbs-file
	./build/test-cm-extended-verbs-file
sanitize-cm-extended-verbs-file: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_EXTENDED_VERBS_FILE_SRC) tests/test_cm_extended_verbs_file.c -o build/test-cm-extended-verbs-file-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-extended-verbs-file-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Comanche055 listing build).
cm-extended-verbs-file-differential:
	python3 tools/interp_routine_differential.py cm-extended-verbs-file --cases 512 --freeze
	git diff --exit-code -- tests/cm_extended_verbs_file_vectors.h tests/cm_extended_verbs_file_c64_vectors.h tools/mk/cm-extended-verbs-file.groups
# The differential again with the oracle tracing the words it executes
# and reads (tools/oracle_trace), then every instruction of the file checked
# executed (interpretive code: read by the interpreter).
.PHONY: cm-extended-verbs-file-closure
cm-extended-verbs-file-closure:
	rm -f build/cm-extended-verbs-file-executed.txt build/cm-extended-verbs-file-read.txt build/interp-routine-reference build/interp-routine-reference-flight-device
	PATH=$(CURDIR)/tools/oracle_trace:$$PATH AGC_EXECUTED_WORDS=$(CURDIR)/build/cm-extended-verbs-file-executed.txt AGC_READ_WORDS=$(CURDIR)/build/cm-extended-verbs-file-read.txt $(MAKE) --no-print-directory cm-extended-verbs-file-differential
	rm -f build/interp-routine-reference build/interp-routine-reference-flight-device
	python3 tools/interp_executed_closure.py cm-extended-verbs-file
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/cm-extended-verbs-file.groups
CM_EXTENDED_VERBS_FILE_C64_PRGS = $(foreach g,$(CM_EXTENDED_VERBS_FILE_GROUPS),build/cm-extended-verbs-file-$(g).prg)
CM_EXTENDED_VERBS_FILE_C64_SRC = src/cm_extended_verbs_file.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/cm_extended_verbs_file_target.c
c64-cm-extended-verbs-file: $(CM_EXTENDED_VERBS_FILE_C64_PRGS)
build/cm-extended-verbs-file-%.prg: $(CM_EXTENDED_VERBS_FILE_SRC) platform/c64/cm_extended_verbs_file_diagnostic.c tools/mk/cm-extended-verbs-file.groups $(CM_EXTENDED_VERBS_FILE_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CM_EXTENDED_VERBS_FILE_MOS_FLAGS) $(CM_EXTENDED_VERBS_FILE_GROUP_FLAGS_$*) -Wl,-Map,build/cm-extended-verbs-file-$*.map $(CM_EXTENDED_VERBS_FILE_C64_SRC) platform/c64/cm_extended_verbs_file_diagnostic.c -o $@
vice-cm-extended-verbs-file: $(CM_EXTENDED_VERBS_FILE_C64_PRGS)
	for p in $(CM_EXTENDED_VERBS_FILE_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
