# SPDX-License-Identifier: MIT
# Milestone cm-exec-idle (lane A): Comanche055 DUMMYJOB, ADVAN, NUCHANG2,
# SUPDXCHZ and the SVCT3 end task as native routines of the control-transfer
# ABI (cm_exec_idle_routines).
CM_EXI_SRC = src/cm_exec_idle.c src/cm_idle_dispatch.c src/cm_idle_recheck.c src/cm_super_dispatch.c src/cm_svct3.c src/cm_dispatch.c src/cm_executive.c src/cm_native_image.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/cm_exec_idle_cases.c tests/cm_idle_dispatch_cases.c tests/cm_idle_recheck_cases.c tests/cm_super_dispatch_cases.c tests/cm_svct3_cases.c tests/cm_basic_finish_cases.c tests/cm_context_change_cases.c tests/cm_admission_complete_cases.c
CM_EXI_HDR = include/cm_exec_routines.h include/agc_transfer.h tests/cm_exec_idle_cases.h
CM_EXI_FLAGS = -ffunction-sections -fdata-sections -Wl,--gc-sections
.PHONY: test-cm-exec-idle sanitize-cm-exec-idle c64-cm-exec-idle vice-cm-exec-idle cm-exec-idle-differential
all: build/test-cm-exec-idle
test: test-cm-exec-idle
sanitize: sanitize-cm-exec-idle
build/test-cm-exec-idle: $(CM_EXI_SRC) tests/test_cm_exec_idle.c tests/cm_exec_idle_vectors.h $(CM_EXI_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_EXI_FLAGS) $(CM_EXI_SRC) tests/test_cm_exec_idle.c -o $@
test-cm-exec-idle: build/test-cm-exec-idle
	./build/test-cm-exec-idle
sanitize-cm-exec-idle: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(CM_EXI_FLAGS) -fsanitize=address,undefined $(CM_EXI_SRC) tests/test_cm_exec_idle.c -o build/test-cm-exec-idle-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-exec-idle-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Comanche055 listing build).
build/cm-exec-idle-native: $(CM_EXI_SRC) platform/host/cm_exec_idle_snapshot.c $(CM_EXI_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_EXI_FLAGS) $(CM_EXI_SRC) platform/host/cm_exec_idle_snapshot.c -o $@
cm-exec-idle-differential: build/cm-exec-idle-native
	python3 tools/cm_exec_idle_freeze.py
	git diff --exit-code -- tests/cm_exec_idle_vectors.h tests/cm_exec_idle_c64_vectors.h
c64-cm-exec-idle: build/cm-exec-idle.prg
build/cm-exec-idle.prg: $(CM_EXI_SRC) platform/c64/cm_exec_idle_diagnostic.c tests/cm_exec_idle_c64_vectors.h $(CM_EXI_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(CM_EXI_FLAGS) -Wl,-Map,build/cm-exec-idle.map $(CM_EXI_SRC) platform/c64/cm_exec_idle_diagnostic.c -o $@
vice-cm-exec-idle: build/cm-exec-idle.prg
	python3 tools/vice_run.py build/cm-exec-idle.prg --cycles 3000000000 --timeout 1200
