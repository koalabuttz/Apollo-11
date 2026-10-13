# SPDX-License-Identifier: MIT
# Milestone cm-v37 (lane A2): Comanche055 FRESH_START_AND_RESTART bank 04 (V37, R00, P00, INITSUB)
# and bank 20 (VAC5STOR) as native routines of the control-transfer ABI.
CM_V37_SRC = src/cm_v37.c src/cm_fresh_start.c src/agc_image_ops.c src/agc_hardware_divide.c src/cm_interbank_routines.c src/cm_interbank_idle.c src/cm_interbank_complete.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/cm_v37_cases.c
CM_V37_HDR = include/cm_v37.h include/cm_fresh_start.h include/cm_interbank_routines.h include/cm_interbank_complete.h include/agc_transfer.h include/agc_image_ops.h src/cm_v37_operands.inc src/cm_fresh_start_operands.inc tests/cm_v37_cases.h
.PHONY: test-cm-v37 sanitize-cm-v37 c64-cm-v37 vice-cm-v37 cm-v37-differential
all: build/test-cm-v37
test: test-cm-v37
sanitize: sanitize-cm-v37
build/test-cm-v37: $(CM_V37_SRC) tests/test_cm_v37.c tests/cm_v37_vectors.h tests/cm_v37_c64_vectors.h $(CM_V37_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_V37_SRC) tests/test_cm_v37.c -o $@
test-cm-v37: build/test-cm-v37
	python3 tools/file_operands_generate.py cm-v37 --check
	python3 tools/file_operands_generate.py cm-fresh-start --check
	./build/test-cm-v37
sanitize-cm-v37: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_V37_SRC) tests/test_cm_v37.c -o build/test-cm-v37-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-v37-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Comanche055 listing build).
build/cm-v37-native: $(CM_V37_SRC) platform/host/cm_v37_snapshot.c $(CM_V37_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_V37_SRC) platform/host/cm_v37_snapshot.c -o $@
cm-v37-differential: build/cm-v37-native
	python3 tools/cm_v37_freeze.py
	git diff --exit-code -- tests/cm_v37_vectors.h tests/cm_v37_c64_vectors.h
c64-cm-v37: build/cm-v37.prg
build/cm-v37.prg: $(CM_V37_SRC) platform/c64/cm_v37_diagnostic.c tests/cm_v37_c64_vectors.h $(CM_V37_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-v37.map $(CM_V37_SRC) platform/c64/cm_v37_diagnostic.c -o $@
vice-cm-v37: build/cm-v37.prg
	python3 tools/vice_run.py build/cm-v37.prg --cycles 8000000000 --timeout 2400
