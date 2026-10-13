# SPDX-License-Identifier: MIT
# Milestone cm-fresh-start (lane A2): Comanche055 FRESH_START_AND_RESTART bank 05 and GOTOPOOH (fresh start,
# restart, ENEMA) as native routines of the control-transfer ABI.
CM_FS_SRC = src/cm_fresh_start.c src/agc_image_ops.c src/agc_hardware_divide.c src/cm_interbank_routines.c src/cm_interbank_idle.c src/cm_interbank_complete.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/cm_fresh_start_cases.c
CM_FS_HDR = include/cm_fresh_start.h include/cm_interbank_routines.h include/cm_interbank_complete.h include/agc_transfer.h include/agc_image_ops.h src/cm_fresh_start_operands.inc tests/cm_fresh_start_cases.h
.PHONY: test-cm-fresh-start sanitize-cm-fresh-start c64-cm-fresh-start vice-cm-fresh-start cm-fresh-start-differential
all: build/test-cm-fresh-start
test: test-cm-fresh-start
sanitize: sanitize-cm-fresh-start
build/test-cm-fresh-start: $(CM_FS_SRC) tests/test_cm_fresh_start.c tests/cm_fresh_start_vectors.h tests/cm_fresh_start_c64_vectors.h $(CM_FS_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_FS_SRC) tests/test_cm_fresh_start.c -o $@
test-cm-fresh-start: build/test-cm-fresh-start
	python3 tools/file_operands_generate.py cm-fresh-start --check
	./build/test-cm-fresh-start
sanitize-cm-fresh-start: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_FS_SRC) tests/test_cm_fresh_start.c -o build/test-cm-fresh-start-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-fresh-start-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Comanche055 listing build).
build/cm-fresh-start-native: $(CM_FS_SRC) platform/host/cm_fresh_start_snapshot.c $(CM_FS_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_FS_SRC) platform/host/cm_fresh_start_snapshot.c -o $@
cm-fresh-start-differential: build/cm-fresh-start-native
	python3 tools/cm_fresh_start_freeze.py
	git diff --exit-code -- tests/cm_fresh_start_vectors.h tests/cm_fresh_start_c64_vectors.h
c64-cm-fresh-start: build/cm-fresh-start.prg
build/cm-fresh-start.prg: $(CM_FS_SRC) platform/c64/cm_fresh_start_diagnostic.c tests/cm_fresh_start_c64_vectors.h $(CM_FS_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-fresh-start.map $(CM_FS_SRC) platform/c64/cm_fresh_start_diagnostic.c -o $@
vice-cm-fresh-start: build/cm-fresh-start.prg
	python3 tools/vice_run.py build/cm-fresh-start.prg --cycles 8000000000 --timeout 2400
