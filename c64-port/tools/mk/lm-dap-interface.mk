# SPDX-License-Identifier: MIT
# Milestone lm-dap-interface (lane F): Luminary099 DAP_INTERFACE_SUBROUTINES
# as native routines of the control-transfer ABI, composed with the native NOVAC.
LM_DAP_SRC = src/lm_dap_interface.c src/lm_exec_routines.c src/lm_admission_image.c src/lm_waitlist_image.c src/waitlist.c src/executive.c src/p66_native.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/lm_dap_interface_cases.c
LM_DAP_HDR = include/lm_dap_interface.h include/lm_exec_routines.h include/agc_transfer.h src/lm_exec_sites.inc tests/lm_dap_interface_cases.h
LM_DAP_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections
.PHONY: test-lm-dap-interface sanitize-lm-dap-interface c64-lm-dap-interface vice-lm-dap-interface lm-dap-interface-differential
all: build/test-lm-dap-interface
test: test-lm-dap-interface
sanitize: sanitize-lm-dap-interface
build/test-lm-dap-interface: $(LM_DAP_SRC) tests/test_lm_dap_interface.c tests/lm_dap_interface_vectors.h $(LM_DAP_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_DAP_FLAGS) $(LM_DAP_SRC) tests/test_lm_dap_interface.c -o $@
test-lm-dap-interface: build/test-lm-dap-interface
	./build/test-lm-dap-interface
	python3 tools/lm_dap_interface_source.py
sanitize-lm-dap-interface: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(LM_DAP_FLAGS) -fsanitize=address,undefined $(LM_DAP_SRC) tests/test_lm_dap_interface.c -o build/test-lm-dap-interface-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-dap-interface-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Luminary099 listing build).
build/lm-dap-interface-native: $(LM_DAP_SRC) platform/host/lm_dap_interface_snapshot.c $(LM_DAP_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_DAP_FLAGS) $(LM_DAP_SRC) platform/host/lm_dap_interface_snapshot.c -o $@
lm-dap-interface-differential: build/lm-dap-interface-native
	python3 tools/lm_dap_interface_freeze.py
	git diff --exit-code -- tests/lm_dap_interface_vectors.h tests/lm_dap_interface_c64_vectors.h
c64-lm-dap-interface: build/lm-dap-interface.prg
build/lm-dap-interface.prg: $(LM_DAP_SRC) platform/c64/lm_dap_interface_diagnostic.c tests/lm_dap_interface_c64_vectors.h $(LM_DAP_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -DP66_NATIVE_DISPATCH_ONLY -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-dap-interface.map $(LM_DAP_SRC) platform/c64/lm_dap_interface_diagnostic.c -o $@
vice-lm-dap-interface: build/lm-dap-interface.prg
	python3 tools/vice_run.py build/lm-dap-interface.prg --cycles 3000000000 --timeout 1200
