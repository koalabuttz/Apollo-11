# SPDX-License-Identifier: MIT
# Milestone cm-tvc-stroke (lane G): Translate Comanche TVCSTROKETEST (V68 stroke test initialization and HACK waveform)
CM_TVC_STROKE_SRC = src/cm_tvc_stroke.c src/agc_image_ops.c src/agc_word.c src/agc_multiword.c src/agc_hardware_divide.c
CM_TVC_STROKE_TEST = tests/test_cm_tvc_stroke.c tests/cm_tvc_stroke_cases.c tests/frozen_words.c
CM_TVC_STROKE_HDR = include/cm_tvc_stroke.h include/agc_image_ops.h tests/cm_tvc_stroke_cases.h tests/cm_tvc_stroke_vectors.h
.PHONY: test-cm-tvc-stroke sanitize-cm-tvc-stroke c64-cm-tvc-stroke vice-cm-tvc-stroke
all: build/test-cm-tvc-stroke
test: test-cm-tvc-stroke
sanitize: sanitize-cm-tvc-stroke
build/test-cm-tvc-stroke: $(CM_TVC_STROKE_SRC) $(CM_TVC_STROKE_TEST) $(CM_TVC_STROKE_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_TVC_STROKE_SRC) $(CM_TVC_STROKE_TEST) -o $@
# All 65536 cases against unchanged Comanche055 in the pinned yaAGC, then the
# frozen rows, then the source map of every emitted original word.
test-cm-tvc-stroke: build/test-cm-tvc-stroke
	./build/test-cm-tvc-stroke
	python3 tools/cm_tvc_stroke_differential.py
	python3 tools/cm_tvc_stroke_source.py
sanitize-cm-tvc-stroke: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer $(CM_TVC_STROKE_SRC) $(CM_TVC_STROKE_TEST) -o build/test-cm-tvc-stroke-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-tvc-stroke-sanitize
	python3 tools/cm_tvc_stroke_differential.py --sanitize
c64-cm-tvc-stroke: build/cm-tvc-stroke.prg
build/cm-tvc-stroke.prg: $(CM_TVC_STROKE_SRC) $(CM_TVC_STROKE_TEST) platform/c64/cm_tvc_stroke_diagnostic.c $(CM_TVC_STROKE_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DCM_TVC_STROKE_CHECK_ONLY -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-tvc-stroke.map $(CM_TVC_STROKE_SRC) $(CM_TVC_STROKE_TEST) platform/c64/cm_tvc_stroke_diagnostic.c -o $@
vice-cm-tvc-stroke: build/cm-tvc-stroke.prg
	python3 tools/vice_run.py build/cm-tvc-stroke.prg
