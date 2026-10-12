# SPDX-License-Identifier: MIT
# Milestone cm-tvc-init (lane G): Translate Comanche TVCINITIALIZE (TVC DAP initialization, CSM/LM switch-over, LOADCOEF, S40.15)
CM_TVC_INIT_SRC = src/cm_tvc_init.c src/agc_image_ops.c src/agc_word.c src/agc_multiword.c src/agc_hardware_divide.c
CM_TVC_INIT_TEST = tests/test_cm_tvc_init.c tests/cm_tvc_init_cases.c tests/frozen_words.c
CM_TVC_INIT_HDR = include/cm_tvc_init.h include/agc_image_ops.h tests/cm_tvc_init_cases.h tests/cm_tvc_init_vectors.h
.PHONY: test-cm-tvc-init sanitize-cm-tvc-init c64-cm-tvc-init vice-cm-tvc-init
all: build/test-cm-tvc-init
test: test-cm-tvc-init
sanitize: sanitize-cm-tvc-init
build/test-cm-tvc-init: $(CM_TVC_INIT_SRC) $(CM_TVC_INIT_TEST) $(CM_TVC_INIT_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_TVC_INIT_SRC) $(CM_TVC_INIT_TEST) -o $@
# All 65536 cases against unchanged Comanche055 in the pinned yaAGC, then the
# frozen rows, then the source map of every emitted original word.
test-cm-tvc-init: build/test-cm-tvc-init
	./build/test-cm-tvc-init
	python3 tools/cm_tvc_init_differential.py
	python3 tools/cm_tvc_init_source.py
sanitize-cm-tvc-init: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer $(CM_TVC_INIT_SRC) $(CM_TVC_INIT_TEST) -o build/test-cm-tvc-init-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-tvc-init-sanitize
	python3 tools/cm_tvc_init_differential.py --sanitize
c64-cm-tvc-init: build/cm-tvc-init.prg
build/cm-tvc-init.prg: $(CM_TVC_INIT_SRC) $(CM_TVC_INIT_TEST) platform/c64/cm_tvc_init_diagnostic.c $(CM_TVC_INIT_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DCM_TVC_INIT_CHECK_ONLY -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-tvc-init.map $(CM_TVC_INIT_SRC) $(CM_TVC_INIT_TEST) platform/c64/cm_tvc_init_diagnostic.c -o $@
vice-cm-tvc-init: build/cm-tvc-init.prg
	python3 tools/vice_run.py build/cm-tvc-init.prg
