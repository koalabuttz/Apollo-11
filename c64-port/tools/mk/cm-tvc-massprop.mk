# SPDX-License-Identifier: MIT
# Milestone cm-tvc-massprop (lane G): Translate Comanche TVCMASSPROP (MASSPROP and FIXCW mass properties)
CM_TVC_MASSPROP_SRC = src/cm_tvc_massprop.c src/agc_image_ops.c src/agc_word.c src/agc_multiword.c src/agc_hardware_divide.c
CM_TVC_MASSPROP_TEST = tests/test_cm_tvc_massprop.c tests/cm_tvc_massprop_cases.c tests/frozen_words.c
CM_TVC_MASSPROP_HDR = include/cm_tvc_massprop.h include/agc_image_ops.h tests/cm_tvc_massprop_cases.h tests/cm_tvc_massprop_vectors.h
.PHONY: test-cm-tvc-massprop sanitize-cm-tvc-massprop c64-cm-tvc-massprop vice-cm-tvc-massprop
all: build/test-cm-tvc-massprop
test: test-cm-tvc-massprop
sanitize: sanitize-cm-tvc-massprop
build/test-cm-tvc-massprop: $(CM_TVC_MASSPROP_SRC) $(CM_TVC_MASSPROP_TEST) $(CM_TVC_MASSPROP_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_TVC_MASSPROP_SRC) $(CM_TVC_MASSPROP_TEST) -o $@
# All 65536 cases against unchanged Comanche055 in the pinned yaAGC, then the
# frozen rows, then the source map of every emitted original word.
test-cm-tvc-massprop: build/test-cm-tvc-massprop
	./build/test-cm-tvc-massprop
	python3 tools/cm_tvc_massprop_differential.py
	python3 tools/cm_tvc_massprop_source.py
	python3 tools/cm_flight_assembly_comparison.py cm-tvc-massprop TVCMASSPROP.agc
sanitize-cm-tvc-massprop: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer $(CM_TVC_MASSPROP_SRC) $(CM_TVC_MASSPROP_TEST) -o build/test-cm-tvc-massprop-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-tvc-massprop-sanitize
	python3 tools/cm_tvc_massprop_differential.py --sanitize
c64-cm-tvc-massprop: build/cm-tvc-massprop.prg
build/cm-tvc-massprop.prg: $(CM_TVC_MASSPROP_SRC) $(CM_TVC_MASSPROP_TEST) platform/c64/cm_tvc_massprop_diagnostic.c $(CM_TVC_MASSPROP_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DCM_TVC_MASSPROP_CHECK_ONLY -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-tvc-massprop.map $(CM_TVC_MASSPROP_SRC) $(CM_TVC_MASSPROP_TEST) platform/c64/cm_tvc_massprop_diagnostic.c -o $@
vice-cm-tvc-massprop: build/cm-tvc-massprop.prg
	python3 tools/vice_run.py build/cm-tvc-massprop.prg
