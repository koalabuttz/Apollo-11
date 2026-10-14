# SPDX-License-Identifier: MIT
# Milestone cm-interpreter (Interpreter lane): the Comanche055 INTERPRETER routines compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
CM_INTERPRETER_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
CM_INTERPRETER_SRC = src/cm_interpreter.c tests/cm_interpreter_target.c src/rope_image_host.c $(CM_INTERPRETER_COMMON)
CM_INTERPRETER_HDR = include/cm_interpreter.h include/rope_image.h tests/interp_case.h tests/cm_interpreter_vectors.h tests/cm_interpreter_c64_vectors.h
CM_INTERPRETER_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -DIRC_CHANNELS
.PHONY: test-cm-interpreter sanitize-cm-interpreter c64-cm-interpreter vice-cm-interpreter cm-interpreter-differential
all: build/test-cm-interpreter
test: test-cm-interpreter
sanitize: sanitize-cm-interpreter
build/test-cm-interpreter: $(CM_INTERPRETER_SRC) tests/test_cm_interpreter.c $(CM_INTERPRETER_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_INTERPRETER_SRC) tests/test_cm_interpreter.c -o $@
# DMP, POLY and the others read their callers' words from the rope image:
# the host test loads the yaYUL binary, the C64 reads the REU's.
test-cm-interpreter: build/test-cm-interpreter build/fixed-data/Comanche055/MAIN.agc.bin
	./build/test-cm-interpreter
sanitize-cm-interpreter: build/fixed-data/Comanche055/MAIN.agc.bin | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_INTERPRETER_SRC) tests/test_cm_interpreter.c -o build/test-cm-interpreter-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-interpreter-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Comanche055 listing build).
cm-interpreter-differential:
	python3 tools/interp_routine_differential.py cm-interpreter --cases 512 --freeze
	git diff --exit-code -- tests/cm_interpreter_vectors.h tests/cm_interpreter_c64_vectors.h tools/mk/cm-interpreter.groups
# The differential again with the oracle tracing the words it executes
# and reads (tools/oracle_trace), then every instruction of the file checked
# executed (interpretive code: read by the interpreter). The core's words are the core's
# evidence.
.PHONY: cm-interpreter-closure
cm-interpreter-closure:
	rm -f build/cm-interpreter-executed.txt build/cm-interpreter-read.txt build/interp-routine-reference build/interp-routine-reference-flight-device
	PATH=$(CURDIR)/tools/oracle_trace:$$PATH AGC_EXECUTED_WORDS=$(CURDIR)/build/cm-interpreter-executed.txt AGC_READ_WORDS=$(CURDIR)/build/cm-interpreter-read.txt $(MAKE) --no-print-directory cm-interpreter-differential
	rm -f build/interp-routine-reference build/interp-routine-reference-flight-device
	python3 tools/interp_executed_closure.py cm-interpreter
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/cm-interpreter.groups
CM_INTERPRETER_C64_PRGS = $(foreach g,$(CM_INTERPRETER_GROUPS),build/cm-interpreter-$(g).prg)
CM_INTERPRETER_C64_SRC = src/cm_interpreter.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/cm_interpreter_target.c src/overlay_reu.c platform/c64/overlay_reu_platform.c platform/c64/rope_image_reu.c
c64-cm-interpreter: $(CM_INTERPRETER_C64_PRGS)
build/cm-interpreter-%.prg: $(CM_INTERPRETER_SRC) platform/c64/cm_interpreter_diagnostic.c tools/mk/cm-interpreter.groups $(CM_INTERPRETER_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CM_INTERPRETER_MOS_FLAGS) $(CM_INTERPRETER_GROUP_FLAGS_$*) -Wl,-Map,build/cm-interpreter-$*.map $(CM_INTERPRETER_C64_SRC) platform/c64/cm_interpreter_diagnostic.c -o $@
vice-cm-interpreter: $(CM_INTERPRETER_C64_PRGS) build/self-check.reu
	for p in $(CM_INTERPRETER_C64_PRGS); do python3 tools/vice_run.py $$p --reu build/self-check.reu --cycles 3000000000 --timeout 1200 || exit 1; done
