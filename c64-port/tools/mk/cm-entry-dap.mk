# SPDX-License-Identifier: MIT
# Milestone cm-entry-dap (lane G): Comanche055 CM_ENTRY_DIGITAL_AUTOPILOT compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
CM_ENTRY_DAP_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
CM_ENTRY_DAP_SRC = src/cm_entry_dap.c src/agc_fast_ops.c src/agc_fast_window.c tests/cm_entry_dap_target.c $(CM_ENTRY_DAP_COMMON)
CM_ENTRY_DAP_HDR = include/cm_entry_dap.h include/agc_fast_ops.h include/agc_fast_window.h include/agc_image_thin.h include/agc_fast_redirect.h tests/interp_case.h tests/cm_entry_dap_vectors.h tests/cm_entry_dap_c64_vectors.h
CM_ENTRY_DAP_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -DIRC_CHANNELS -DAGC_CHANNELS_HW=irc_cm_hw
.PHONY: test-cm-entry-dap sanitize-cm-entry-dap c64-cm-entry-dap vice-cm-entry-dap cm-entry-dap-differential
all: build/test-cm-entry-dap
test: test-cm-entry-dap
sanitize: sanitize-cm-entry-dap
build/test-cm-entry-dap: $(CM_ENTRY_DAP_SRC) tests/test_cm_entry_dap.c $(CM_ENTRY_DAP_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_ENTRY_DAP_SRC) tests/test_cm_entry_dap.c -o $@
test-cm-entry-dap: build/test-cm-entry-dap
	./build/test-cm-entry-dap
sanitize-cm-entry-dap: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_ENTRY_DAP_SRC) tests/test_cm_entry_dap.c -o build/test-cm-entry-dap-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-entry-dap-sanitize
# The hot table (spec hot_points, the flight store's entry pack) calls the
# full table's functions for its keys.
.PHONY: test-cm-entry-dap-hot
all: build/test-cm-entry-dap-hot
test: test-cm-entry-dap-hot
build/test-cm-entry-dap-hot: $(CM_ENTRY_DAP_SRC) tests/test_cm_entry_dap_hot.c $(CM_ENTRY_DAP_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_ENTRY_DAP_SRC) tests/test_cm_entry_dap_hot.c -o $@
test-cm-entry-dap-hot: build/test-cm-entry-dap-hot
	./build/test-cm-entry-dap-hot
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Comanche055 listing build).
cm-entry-dap-differential:
	python3 tools/interp_routine_differential.py cm-entry-dap --cases 256 --freeze
	git diff --exit-code -- tests/cm_entry_dap_vectors.h tests/cm_entry_dap_c64_vectors.h tools/mk/cm-entry-dap.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/cm-entry-dap.groups
CM_ENTRY_DAP_C64_PRGS = $(foreach g,$(CM_ENTRY_DAP_GROUPS),build/cm-entry-dap-$(g).prg)
CM_ENTRY_DAP_C64_SRC = src/cm_entry_dap.c src/agc_fast_ops.c src/agc_fast_window.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/cm_entry_dap_target.c
c64-cm-entry-dap: $(CM_ENTRY_DAP_C64_PRGS)
build/cm-entry-dap-%.prg: $(CM_ENTRY_DAP_SRC) platform/c64/cm_entry_dap_diagnostic.c tools/mk/cm-entry-dap.groups $(CM_ENTRY_DAP_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CM_ENTRY_DAP_MOS_FLAGS) $(CM_ENTRY_DAP_GROUP_FLAGS_$*) -Wl,-Map,build/cm-entry-dap-$*.map $(CM_ENTRY_DAP_C64_SRC) platform/c64/cm_entry_dap_diagnostic.c -o $@
vice-cm-entry-dap: $(CM_ENTRY_DAP_C64_PRGS)
	for p in $(CM_ENTRY_DAP_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
