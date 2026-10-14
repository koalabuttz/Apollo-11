# SPDX-License-Identifier: MIT
# Milestone cm-rcs-dap-exec (lane A): the Comanche055 RCS-CSM_DAP_EXECUTIVE_PROGRAMS job (AMBGUPDT) compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
CM_RCS_DAP_EXEC_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
CM_RCS_DAP_EXEC_SRC = src/cm_rcs_dap_exec.c tests/cm_rcs_dap_exec_target.c $(CM_RCS_DAP_EXEC_COMMON)
CM_RCS_DAP_EXEC_HDR = include/cm_rcs_dap_exec.h tests/interp_case.h tests/cm_rcs_dap_exec_vectors.h tests/cm_rcs_dap_exec_c64_vectors.h
CM_RCS_DAP_EXEC_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -DIRC_CHANNELS
.PHONY: test-cm-rcs-dap-exec sanitize-cm-rcs-dap-exec c64-cm-rcs-dap-exec vice-cm-rcs-dap-exec cm-rcs-dap-exec-differential
all: build/test-cm-rcs-dap-exec
test: test-cm-rcs-dap-exec
sanitize: sanitize-cm-rcs-dap-exec
build/test-cm-rcs-dap-exec: $(CM_RCS_DAP_EXEC_SRC) tests/test_cm_rcs_dap_exec.c $(CM_RCS_DAP_EXEC_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_RCS_DAP_EXEC_SRC) tests/test_cm_rcs_dap_exec.c -o $@
test-cm-rcs-dap-exec: build/test-cm-rcs-dap-exec
	./build/test-cm-rcs-dap-exec
sanitize-cm-rcs-dap-exec: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_RCS_DAP_EXEC_SRC) tests/test_cm_rcs_dap_exec.c -o build/test-cm-rcs-dap-exec-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-rcs-dap-exec-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Comanche055 listing build).
cm-rcs-dap-exec-differential:
	python3 tools/interp_routine_differential.py cm-rcs-dap-exec --cases 512 --freeze
	git diff --exit-code -- tests/cm_rcs_dap_exec_vectors.h tests/cm_rcs_dap_exec_c64_vectors.h tools/mk/cm-rcs-dap-exec.groups
# The differential again with the oracle tracing the words it executes
# (tools/oracle_trace), then every instruction of the file checked executed.
.PHONY: cm-rcs-dap-exec-closure
cm-rcs-dap-exec-closure:
	rm -f build/cm-rcs-dap-exec-executed.txt build/interp-routine-reference build/interp-routine-reference-flight-device
	PATH=$(CURDIR)/tools/oracle_trace:$$PATH AGC_EXECUTED_WORDS=$(CURDIR)/build/cm-rcs-dap-exec-executed.txt $(MAKE) --no-print-directory cm-rcs-dap-exec-differential
	rm -f build/interp-routine-reference build/interp-routine-reference-flight-device
	python3 tools/interp_executed_closure.py cm-rcs-dap-exec
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/cm-rcs-dap-exec.groups
CM_RCS_DAP_EXEC_C64_PRGS = $(foreach g,$(CM_RCS_DAP_EXEC_GROUPS),build/cm-rcs-dap-exec-$(g).prg)
CM_RCS_DAP_EXEC_C64_SRC = src/cm_rcs_dap_exec.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/cm_rcs_dap_exec_target.c
c64-cm-rcs-dap-exec: $(CM_RCS_DAP_EXEC_C64_PRGS)
build/cm-rcs-dap-exec-%.prg: $(CM_RCS_DAP_EXEC_SRC) platform/c64/cm_rcs_dap_exec_diagnostic.c tools/mk/cm-rcs-dap-exec.groups $(CM_RCS_DAP_EXEC_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CM_RCS_DAP_EXEC_MOS_FLAGS) $(CM_RCS_DAP_EXEC_GROUP_FLAGS_$*) -Wl,-Map,build/cm-rcs-dap-exec-$*.map $(CM_RCS_DAP_EXEC_C64_SRC) platform/c64/cm_rcs_dap_exec_diagnostic.c -o $@
vice-cm-rcs-dap-exec: $(CM_RCS_DAP_EXEC_C64_PRGS)
	for p in $(CM_RCS_DAP_EXEC_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
