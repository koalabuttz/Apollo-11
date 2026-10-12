# SPDX-License-Identifier: MIT
# Milestone cm-tvc-restarts (lane G): Translate Comanche TVCRESTARTS (TVC restart package, outcounter enables, EXRSTRT)
CM_TVC_RESTARTS_SRC = src/cm_tvc_restarts.c src/agc_image_ops.c src/agc_word.c src/agc_multiword.c src/agc_hardware_divide.c
CM_TVC_RESTARTS_TEST = tests/test_cm_tvc_restarts.c tests/cm_tvc_restarts_cases.c tests/frozen_words.c
CM_TVC_RESTARTS_HDR = include/cm_tvc_restarts.h include/agc_image_ops.h tests/cm_tvc_restarts_cases.h tests/cm_tvc_restarts_vectors.h
.PHONY: test-cm-tvc-restarts sanitize-cm-tvc-restarts c64-cm-tvc-restarts vice-cm-tvc-restarts
all: build/test-cm-tvc-restarts
test: test-cm-tvc-restarts
sanitize: sanitize-cm-tvc-restarts
build/test-cm-tvc-restarts: $(CM_TVC_RESTARTS_SRC) $(CM_TVC_RESTARTS_TEST) $(CM_TVC_RESTARTS_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_TVC_RESTARTS_SRC) $(CM_TVC_RESTARTS_TEST) -o $@
# All 65536 cases against unchanged Comanche055 in the pinned yaAGC, then the
# frozen rows, then the source map of every emitted original word.
test-cm-tvc-restarts: build/test-cm-tvc-restarts
	./build/test-cm-tvc-restarts
	python3 tools/cm_tvc_restarts_differential.py
	python3 tools/cm_tvc_restarts_source.py
	python3 tools/cm_flight_assembly_comparison.py cm-tvc-restarts TVCRESTARTS.agc
sanitize-cm-tvc-restarts: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer $(CM_TVC_RESTARTS_SRC) $(CM_TVC_RESTARTS_TEST) -o build/test-cm-tvc-restarts-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-tvc-restarts-sanitize
	python3 tools/cm_tvc_restarts_differential.py --sanitize
c64-cm-tvc-restarts: build/cm-tvc-restarts.prg
build/cm-tvc-restarts.prg: $(CM_TVC_RESTARTS_SRC) $(CM_TVC_RESTARTS_TEST) platform/c64/cm_tvc_restarts_diagnostic.c $(CM_TVC_RESTARTS_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DCM_TVC_RESTARTS_CHECK_ONLY -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-tvc-restarts.map $(CM_TVC_RESTARTS_SRC) $(CM_TVC_RESTARTS_TEST) platform/c64/cm_tvc_restarts_diagnostic.c -o $@
vice-cm-tvc-restarts: build/cm-tvc-restarts.prg
	python3 tools/vice_run.py build/cm-tvc-restarts.prg
