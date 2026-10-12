# SPDX-License-Identifier: MIT
# Milestone cm-tvc-daps (lane G): Translate Comanche TVCDAPS (CM pitch and yaw TVC autopilot)
CM_TVC_DAPS_SRC = src/cm_tvc_daps.c src/agc_image_ops.c src/agc_word.c src/agc_multiword.c src/agc_hardware_divide.c
CM_TVC_DAPS_TEST = tests/test_cm_tvc_daps.c tests/cm_tvc_daps_cases.c tests/frozen_words.c
CM_TVC_DAPS_HDR = include/cm_tvc_daps.h include/agc_image_ops.h tests/cm_tvc_daps_cases.h tests/cm_tvc_daps_vectors.h
.PHONY: test-cm-tvc-daps sanitize-cm-tvc-daps c64-cm-tvc-daps vice-cm-tvc-daps
all: build/test-cm-tvc-daps
test: test-cm-tvc-daps
sanitize: sanitize-cm-tvc-daps
build/test-cm-tvc-daps: $(CM_TVC_DAPS_SRC) $(CM_TVC_DAPS_TEST) $(CM_TVC_DAPS_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_TVC_DAPS_SRC) $(CM_TVC_DAPS_TEST) -o $@
# All 65536 cases against unchanged Comanche055 in the pinned yaAGC, then the
# frozen rows, then the source map of every emitted original word.
test-cm-tvc-daps: build/test-cm-tvc-daps
	./build/test-cm-tvc-daps
	python3 tools/cm_tvc_daps_differential.py
	python3 tools/cm_tvc_daps_source.py
	python3 tools/cm_flight_assembly_comparison.py cm-tvc-daps TVCDAPS.agc
sanitize-cm-tvc-daps: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer $(CM_TVC_DAPS_SRC) $(CM_TVC_DAPS_TEST) -o build/test-cm-tvc-daps-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-tvc-daps-sanitize
	python3 tools/cm_tvc_daps_differential.py --sanitize
c64-cm-tvc-daps: build/cm-tvc-daps.prg
build/cm-tvc-daps.prg: $(CM_TVC_DAPS_SRC) $(CM_TVC_DAPS_TEST) platform/c64/cm_tvc_daps_diagnostic.c $(CM_TVC_DAPS_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DCM_TVC_DAPS_CHECK_ONLY -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-tvc-daps.map $(CM_TVC_DAPS_SRC) $(CM_TVC_DAPS_TEST) platform/c64/cm_tvc_daps_diagnostic.c -o $@
vice-cm-tvc-daps: build/cm-tvc-daps.prg
	python3 tools/vice_run.py build/cm-tvc-daps.prg
