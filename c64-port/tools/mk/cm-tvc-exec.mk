# SPDX-License-Identifier: MIT
# Milestone cm-tvc-exec (lane G): Translate Comanche TVCEXECUTIVE (TVCEXEC task, roll ladders, gain/mass and trim updates)
CM_TVC_EXEC_SRC = src/cm_tvc_exec.c src/agc_image_ops.c src/agc_word.c src/agc_multiword.c src/agc_hardware_divide.c
CM_TVC_EXEC_TEST = tests/test_cm_tvc_exec.c tests/cm_tvc_exec_cases.c tests/frozen_words.c
CM_TVC_EXEC_HDR = include/cm_tvc_exec.h include/agc_image_ops.h tests/cm_tvc_exec_cases.h tests/cm_tvc_exec_vectors.h
.PHONY: test-cm-tvc-exec sanitize-cm-tvc-exec c64-cm-tvc-exec vice-cm-tvc-exec
all: build/test-cm-tvc-exec
test: test-cm-tvc-exec
sanitize: sanitize-cm-tvc-exec
build/test-cm-tvc-exec: $(CM_TVC_EXEC_SRC) $(CM_TVC_EXEC_TEST) $(CM_TVC_EXEC_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_TVC_EXEC_SRC) $(CM_TVC_EXEC_TEST) -o $@
# All 65536 cases against unchanged Comanche055 in the pinned yaAGC, then the
# frozen rows, then the source map of every emitted original word.
test-cm-tvc-exec: build/test-cm-tvc-exec
	./build/test-cm-tvc-exec
	python3 tools/cm_tvc_exec_differential.py
	python3 tools/cm_tvc_exec_source.py
	python3 tools/cm_flight_assembly_comparison.py cm-tvc-exec TVCEXECUTIVE.agc
sanitize-cm-tvc-exec: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer $(CM_TVC_EXEC_SRC) $(CM_TVC_EXEC_TEST) -o build/test-cm-tvc-exec-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-tvc-exec-sanitize
	python3 tools/cm_tvc_exec_differential.py --sanitize
c64-cm-tvc-exec: build/cm-tvc-exec.prg
build/cm-tvc-exec.prg: $(CM_TVC_EXEC_SRC) $(CM_TVC_EXEC_TEST) platform/c64/cm_tvc_exec_diagnostic.c $(CM_TVC_EXEC_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DCM_TVC_EXEC_CHECK_ONLY -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-tvc-exec.map $(CM_TVC_EXEC_SRC) $(CM_TVC_EXEC_TEST) platform/c64/cm_tvc_exec_diagnostic.c -o $@
vice-cm-tvc-exec: build/cm-tvc-exec.prg
	python3 tools/vice_run.py build/cm-tvc-exec.prg
