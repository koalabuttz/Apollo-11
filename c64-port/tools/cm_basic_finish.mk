# SPDX-License-Identifier: MIT
CM_FINISH_CORE = src/cm_basic_finish.c src/cm_dispatch.c src/cm_executive.c src/cm_native_image.c src/agc_word.c tests/cm_basic_finish_cases.c
CM_FINISH_HEADERS = include/cm_basic_finish.h include/cm_dispatch.h include/cm_executive.h include/cm_native_image.h include/overlay_runtime.h tests/cm_basic_finish_cases.h tests/cm_basic_finish_check.h tests/cm_basic_finish_vectors.h
all: build/cm-basic-finish-native
build/cm-basic-finish-native: $(CM_FINISH_CORE) platform/host/cm_basic_finish_snapshot.c $(CM_FINISH_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_FINISH_CORE) platform/host/cm_basic_finish_snapshot.c -o $@
cm-basic-finish-differential: build/cm-basic-finish-native
	python3 tools/cm_basic_finish_differential.py

CM_FINISH_CHECK = tests/cm_basic_finish_check.c
.PHONY: test-cm-basic-finish sanitize-cm-basic-finish c64-cm-basic-finish
test: test-cm-basic-finish
test-cm-basic-finish: build/test-cm-basic-finish
	./build/test-cm-basic-finish
build/test-cm-basic-finish: $(CM_FINISH_CORE) $(CM_FINISH_CHECK) tests/test_cm_basic_finish.c $(CM_FINISH_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_FINISH_CORE) $(CM_FINISH_CHECK) tests/test_cm_basic_finish.c -o $@
sanitize: sanitize-cm-basic-finish
sanitize-cm-basic-finish: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_FINISH_CORE) $(CM_FINISH_CHECK) tests/test_cm_basic_finish.c -o build/test-cm-basic-finish-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-basic-finish-sanitize
c64-cm-basic-finish: build/cm-basic-finish.prg
build/cm-basic-finish.prg: $(CM_FINISH_CORE) $(CM_FINISH_CHECK) platform/c64/cm_basic_finish_diagnostic.c $(CM_FINISH_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-basic-finish.map $(CM_FINISH_CORE) $(CM_FINISH_CHECK) platform/c64/cm_basic_finish_diagnostic.c -o $@
