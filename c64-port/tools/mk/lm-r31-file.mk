# SPDX-License-Identifier: MIT
# Milestone lm-r31-file (lane A): the Luminary099 R31 (rendezvous parameter display) routines compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
LM_R31_FILE_COMMON = src/rtb_angles.c src/lm_normunit.c src/rtb_vector.c src/rtb_unit.c src/guidance_unit_phased.c src/guidance_length_phased.c src/guidance_norm.c src/guidance_product.c src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
LM_R31_FILE_SRC = src/lm_r31_file.c tests/lm_r31_file_target.c $(LM_R31_FILE_COMMON)
LM_R31_FILE_HDR = include/lm_r31_file.h tests/interp_case.h tests/lm_r31_file_vectors.h tests/lm_r31_file_c64_vectors.h
LM_R31_FILE_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -DIRC_CHANNELS -fno-inline -fstatic-stack -mlto-zp=110
.PHONY: test-lm-r31-file sanitize-lm-r31-file c64-lm-r31-file vice-lm-r31-file lm-r31-file-differential
all: build/test-lm-r31-file
test: test-lm-r31-file
sanitize: sanitize-lm-r31-file
build/test-lm-r31-file: $(LM_R31_FILE_SRC) tests/test_lm_r31_file.c $(LM_R31_FILE_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_R31_FILE_SRC) tests/test_lm_r31_file.c -o $@
test-lm-r31-file: build/test-lm-r31-file
	./build/test-lm-r31-file
sanitize-lm-r31-file: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(LM_R31_FILE_SRC) tests/test_lm_r31_file.c -o build/test-lm-r31-file-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-r31-file-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Luminary099 listing build).
lm-r31-file-differential:
	python3 tools/interp_routine_differential.py lm-r31-file --cases 512 --freeze
	git diff --exit-code -- tests/lm_r31_file_vectors.h tests/lm_r31_file_c64_vectors.h tools/mk/lm-r31-file.groups
# The differential again with the oracle tracing the words it executes
# and reads (tools/oracle_trace), then every instruction of the file checked
# executed (interpretive code: read by the interpreter).
.PHONY: lm-r31-file-closure
lm-r31-file-closure:
	rm -f build/lm-r31-file-executed.txt build/lm-r31-file-read.txt build/interp-routine-reference build/interp-routine-reference-flight-device
	PATH=$(CURDIR)/tools/oracle_trace:$$PATH AGC_EXECUTED_WORDS=$(CURDIR)/build/lm-r31-file-executed.txt AGC_READ_WORDS=$(CURDIR)/build/lm-r31-file-read.txt $(MAKE) --no-print-directory lm-r31-file-differential
	rm -f build/interp-routine-reference build/interp-routine-reference-flight-device
	python3 tools/interp_executed_closure.py lm-r31-file
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/lm-r31-file.groups
LM_R31_FILE_C64_PRGS = $(foreach g,$(LM_R31_FILE_GROUPS),build/lm-r31-file-$(g).prg)
LM_R31_FILE_C64_SRC = src/lm_r31_file.c src/rtb_angles.c src/lm_normunit.c src/rtb_vector.c src/rtb_unit.c src/guidance_unit_phased.c src/guidance_length_phased.c src/guidance_norm.c src/guidance_product.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/interp_cut.c tests/lm_r31_file_target.c
c64-lm-r31-file: $(LM_R31_FILE_C64_PRGS)
build/lm-r31-file-%.prg: $(LM_R31_FILE_SRC) platform/c64/lm_r31_file_diagnostic.c tools/mk/lm-r31-file.groups $(LM_R31_FILE_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(LM_R31_FILE_MOS_FLAGS) $(LM_R31_FILE_GROUP_FLAGS_$*) -Wl,-Map,build/lm-r31-file-$*.map $(LM_R31_FILE_C64_SRC) platform/c64/lm_r31_file_diagnostic.c -o $@
vice-lm-r31-file: $(LM_R31_FILE_C64_PRGS)
	for p in $(LM_R31_FILE_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
