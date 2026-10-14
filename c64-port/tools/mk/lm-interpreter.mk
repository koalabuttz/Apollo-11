# SPDX-License-Identifier: MIT
# Milestone lm-interpreter (Interpreter lane): the Luminary099 INTERPRETER routines compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
LM_INTERPRETER_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
LM_INTERPRETER_SRC = src/lm_interpreter.c tests/lm_interpreter_target.c src/rope_image_host.c $(LM_INTERPRETER_COMMON)
LM_INTERPRETER_HDR = include/lm_interpreter.h include/rope_image.h tests/interp_case.h tests/lm_interpreter_vectors.h tests/lm_interpreter_c64_vectors.h
LM_INTERPRETER_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -DIRC_CHANNELS
.PHONY: test-lm-interpreter sanitize-lm-interpreter c64-lm-interpreter vice-lm-interpreter lm-interpreter-differential
all: build/test-lm-interpreter
test: test-lm-interpreter
sanitize: sanitize-lm-interpreter
build/test-lm-interpreter: $(LM_INTERPRETER_SRC) tests/test_lm_interpreter.c $(LM_INTERPRETER_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_INTERPRETER_SRC) tests/test_lm_interpreter.c -o $@
# DMP, POLY and the others read their callers' words from the rope image:
# the host test loads the yaYUL binary, the C64 reads the REU's.
test-lm-interpreter: build/test-lm-interpreter build/fixed-data/Luminary099/MAIN.agc.bin
	./build/test-lm-interpreter
sanitize-lm-interpreter: build/fixed-data/Luminary099/MAIN.agc.bin | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(LM_INTERPRETER_SRC) tests/test_lm_interpreter.c -o build/test-lm-interpreter-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-interpreter-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Luminary099 listing build).
lm-interpreter-differential:
	python3 tools/interp_routine_differential.py lm-interpreter --cases 512 --freeze
	git diff --exit-code -- tests/lm_interpreter_vectors.h tests/lm_interpreter_c64_vectors.h tools/mk/lm-interpreter.groups
# The differential again with the oracle tracing the words it executes
# and reads (tools/oracle_trace), then every instruction of the file checked
# executed (interpretive code: read by the interpreter). DPAGREE's two words
# cannot be reached (named below); the core's words are the core's evidence.
.PHONY: lm-interpreter-closure
lm-interpreter-closure:
	rm -f build/lm-interpreter-executed.txt build/lm-interpreter-read.txt build/interp-routine-reference build/interp-routine-reference-flight-device
	PATH=$(CURDIR)/tools/oracle_trace:$$PATH AGC_EXECUTED_WORDS=$(CURDIR)/build/lm-interpreter-executed.txt AGC_READ_WORDS=$(CURDIR)/build/lm-interpreter-read.txt $(MAKE) --no-print-directory lm-interpreter-differential
	rm -f build/interp-routine-reference build/interp-routine-reference-flight-device
	python3 tools/interp_executed_closure.py lm-interpreter --unreached '03,3255=POLY counts POLYCNT down to +0 and returns there, so CCS POLYCNT never takes its negative exit into DPAGREE (no other code enters DPAGREE in Luminary099)' '03,3256=as 03,3255: reached only through DPAGREE'
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/lm-interpreter.groups
LM_INTERPRETER_C64_PRGS = $(foreach g,$(LM_INTERPRETER_GROUPS),build/lm-interpreter-$(g).prg)
LM_INTERPRETER_C64_SRC = src/lm_interpreter.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/lm_interpreter_target.c src/overlay_reu.c platform/c64/overlay_reu_platform.c platform/c64/rope_image_reu.c
c64-lm-interpreter: $(LM_INTERPRETER_C64_PRGS)
build/lm-interpreter-%.prg: $(LM_INTERPRETER_SRC) platform/c64/lm_interpreter_diagnostic.c tools/mk/lm-interpreter.groups $(LM_INTERPRETER_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(LM_INTERPRETER_MOS_FLAGS) $(LM_INTERPRETER_GROUP_FLAGS_$*) -Wl,-Map,build/lm-interpreter-$*.map $(LM_INTERPRETER_C64_SRC) platform/c64/lm_interpreter_diagnostic.c -o $@
vice-lm-interpreter: $(LM_INTERPRETER_C64_PRGS) build/self-check.reu
	for p in $(LM_INTERPRETER_C64_PRGS); do python3 tools/vice_run.py $$p --reu build/self-check.reu --cycles 3000000000 --timeout 1200 || exit 1; done
