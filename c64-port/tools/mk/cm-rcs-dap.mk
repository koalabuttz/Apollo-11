# SPDX-License-Identifier: MIT
# Milestone cm-rcs-dap (lane G): Comanche055 RCS-CSM_DIGITAL_AUTOPILOT compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
CM_RCS_DAP_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
CM_RCS_DAP_SRC = src/cm_rcs_dap.c tests/cm_rcs_dap_target.c $(CM_RCS_DAP_COMMON)
CM_RCS_DAP_HDR = include/cm_rcs_dap.h tests/interp_case.h tests/cm_rcs_dap_vectors.h tests/cm_rcs_dap_c64_vectors.h
CM_RCS_DAP_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -DIRC_CHANNELS
.PHONY: test-cm-rcs-dap sanitize-cm-rcs-dap c64-cm-rcs-dap vice-cm-rcs-dap cm-rcs-dap-differential
all: build/test-cm-rcs-dap
test: test-cm-rcs-dap
sanitize: sanitize-cm-rcs-dap
build/test-cm-rcs-dap: $(CM_RCS_DAP_SRC) tests/test_cm_rcs_dap.c $(CM_RCS_DAP_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_RCS_DAP_SRC) tests/test_cm_rcs_dap.c -o $@
test-cm-rcs-dap: build/test-cm-rcs-dap
	./build/test-cm-rcs-dap
sanitize-cm-rcs-dap: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_RCS_DAP_SRC) tests/test_cm_rcs_dap.c -o build/test-cm-rcs-dap-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-rcs-dap-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Comanche055 listing build).
cm-rcs-dap-differential:
	python3 tools/interp_routine_differential.py cm-rcs-dap --cases 256 --freeze
	git diff --exit-code -- tests/cm_rcs_dap_vectors.h tests/cm_rcs_dap_c64_vectors.h tools/mk/cm-rcs-dap.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/cm-rcs-dap.groups
CM_RCS_DAP_C64_PRGS = $(foreach g,$(CM_RCS_DAP_GROUPS),build/cm-rcs-dap-$(g).prg)
CM_RCS_DAP_C64_SRC = src/cm_rcs_dap.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/cm_rcs_dap_target.c
c64-cm-rcs-dap: $(CM_RCS_DAP_C64_PRGS)
build/cm-rcs-dap-%.prg: $(CM_RCS_DAP_SRC) platform/c64/cm_rcs_dap_diagnostic.c tools/mk/cm-rcs-dap.groups $(CM_RCS_DAP_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CM_RCS_DAP_MOS_FLAGS) $(CM_RCS_DAP_GROUP_FLAGS_$*) -Wl,-Map,build/cm-rcs-dap-$*.map $(CM_RCS_DAP_C64_SRC) platform/c64/cm_rcs_dap_diagnostic.c -o $@
vice-cm-rcs-dap: $(CM_RCS_DAP_C64_PRGS)
	for p in $(CM_RCS_DAP_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
