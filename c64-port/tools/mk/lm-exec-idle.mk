# SPDX-License-Identifier: MIT
# Milestone lm-exec-idle (lane A): Luminary099 DUMMYJOB, ADVAN, NUCHANG2,
# SUPDXCHZ and the SVCT3 end task as native routines of the control-transfer
# ABI (lm_exec_idle_routines).
LM_EXI_SRC = src/lm_exec_idle.c src/lm_idle_dispatch.c src/lm_super_dispatch.c src/lm_svct3.c src/executive.c src/waitlist.c src/p66_native.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/lm_exec_idle_cases.c tests/lm_idle_dispatch_cases.c tests/lm_super_dispatch_cases.c tests/lm_svct3_cases.c tests/lm_context_lifecycle_cases.c tests/lm_admission_image_cases.c tests/lm_priority_cases.c tests/lm_sleep_cases.c tests/throttle_bridge_cases.c
LM_EXI_HDR = include/lm_exec_routines.h include/lm_svct3.h include/agc_transfer.h tests/lm_exec_idle_cases.h
LM_EXI_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections
.PHONY: test-lm-exec-idle sanitize-lm-exec-idle c64-lm-exec-idle vice-lm-exec-idle lm-exec-idle-differential
all: build/test-lm-exec-idle
test: test-lm-exec-idle
sanitize: sanitize-lm-exec-idle
build/test-lm-exec-idle: $(LM_EXI_SRC) tests/test_lm_exec_idle.c tests/lm_exec_idle_vectors.h $(LM_EXI_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_EXI_FLAGS) $(LM_EXI_SRC) tests/test_lm_exec_idle.c -o $@
test-lm-exec-idle: build/test-lm-exec-idle
	./build/test-lm-exec-idle
sanitize-lm-exec-idle: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(LM_EXI_FLAGS) -fsanitize=address,undefined $(LM_EXI_SRC) tests/test_lm_exec_idle.c -o build/test-lm-exec-idle-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-exec-idle-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Luminary099 listing build).
build/lm-exec-idle-native: $(LM_EXI_SRC) platform/host/lm_exec_idle_snapshot.c $(LM_EXI_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_EXI_FLAGS) $(LM_EXI_SRC) platform/host/lm_exec_idle_snapshot.c -o $@
lm-exec-idle-differential: build/lm-exec-idle-native
	python3 tools/lm_exec_idle_freeze.py
	git diff --exit-code -- tests/lm_exec_idle_vectors.h tests/lm_exec_idle_c64_vectors.h
c64-lm-exec-idle: build/lm-exec-idle.prg
build/lm-exec-idle.prg: $(LM_EXI_SRC) platform/c64/lm_exec_idle_diagnostic.c tests/lm_exec_idle_c64_vectors.h $(LM_EXI_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(LM_EXI_FLAGS) -Wl,-Map,build/lm-exec-idle.map $(LM_EXI_SRC) platform/c64/lm_exec_idle_diagnostic.c -o $@
vice-lm-exec-idle: build/lm-exec-idle.prg
	python3 tools/vice_run.py build/lm-exec-idle.prg --cycles 3000000000 --timeout 1200
