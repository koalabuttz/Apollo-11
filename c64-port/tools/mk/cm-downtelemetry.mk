# SPDX-License-Identifier: MIT
# Milestone cm-downtelemetry (lane A2): Comanche055 DOWN-TELEMETRY_PROGRAM
# (DODOWNTM, DNTMGOTO, downlists and erasable dump) as native routines of the
# control-transfer ABI.
CM_DNTM_SRC = src/cm_downtelemetry.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/cm_downtelemetry_cases.c
CM_DNTM_HDR = include/cm_downtelemetry.h include/agc_transfer.h include/agc_image_ops.h src/cm_downtelemetry_data.inc tests/cm_downtelemetry_cases.h
.PHONY: test-cm-downtelemetry sanitize-cm-downtelemetry c64-cm-downtelemetry vice-cm-downtelemetry cm-downtelemetry-differential
all: build/test-cm-downtelemetry
test: test-cm-downtelemetry
sanitize: sanitize-cm-downtelemetry
build/test-cm-downtelemetry: $(CM_DNTM_SRC) tests/test_cm_downtelemetry.c tests/cm_downtelemetry_vectors.h $(CM_DNTM_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_DNTM_SRC) tests/test_cm_downtelemetry.c -o $@
test-cm-downtelemetry: build/test-cm-downtelemetry
	python3 tools/downtelemetry_generate.py --mission Comanche055 --check
	./build/test-cm-downtelemetry
sanitize-cm-downtelemetry: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_DNTM_SRC) tests/test_cm_downtelemetry.c -o build/test-cm-downtelemetry-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-downtelemetry-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Comanche055 listing build).
build/cm-downtelemetry-native: $(CM_DNTM_SRC) platform/host/cm_downtelemetry_snapshot.c $(CM_DNTM_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_DNTM_SRC) platform/host/cm_downtelemetry_snapshot.c -o $@
cm-downtelemetry-differential: build/cm-downtelemetry-native
	python3 tools/cm_downtelemetry_freeze.py
	git diff --exit-code -- tests/cm_downtelemetry_vectors.h tests/cm_downtelemetry_c64_vectors.h
c64-cm-downtelemetry: build/cm-downtelemetry.prg
build/cm-downtelemetry.prg: $(CM_DNTM_SRC) platform/c64/cm_downtelemetry_diagnostic.c tests/cm_downtelemetry_c64_vectors.h $(CM_DNTM_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-downtelemetry.map $(CM_DNTM_SRC) platform/c64/cm_downtelemetry_diagnostic.c -o $@
vice-cm-downtelemetry: build/cm-downtelemetry.prg
	python3 tools/vice_run.py build/cm-downtelemetry.prg --cycles 8000000000 --timeout 2400
