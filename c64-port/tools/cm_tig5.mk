# SPDX-License-Identifier: MIT
CM_TIG5_CORE = src/cm_tig5.c src/cm_phase.c src/cm_native_image.c src/agc_word.c src/agc_multiword.c tests/cm_tig5_cases.c
CM_TIG5_CHECK = tests/cm_tig5_check.c
CM_TIG5_HEADERS = include/cm_tig5.h include/cm_phase.h include/cm_native_image.h include/overlay_runtime.h include/agc_word.h include/agc_multiword.h tests/cm_tig5_cases.h tests/cm_tig5_check.h tests/cm_tig5_vectors.h
.PHONY: cm-tig5-differential test-cm-tig5 sanitize-cm-tig5 c64-cm-tig5
all: build/cm-tig5-native build/test-cm-tig5
build/cm-tig5-native: $(CM_TIG5_CORE) platform/host/cm_tig5_snapshot.c $(CM_TIG5_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_TIG5_CORE) platform/host/cm_tig5_snapshot.c -o $@
cm-tig5-differential: build/cm-tig5-native
	python3 tools/cm_tig5_differential.py
build/test-cm-tig5: $(CM_TIG5_CORE) $(CM_TIG5_CHECK) tests/test_cm_tig5.c $(CM_TIG5_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_TIG5_CORE) $(CM_TIG5_CHECK) tests/test_cm_tig5.c -o $@
test: test-cm-tig5
test-cm-tig5: build/test-cm-tig5
	./build/test-cm-tig5
sanitize: sanitize-cm-tig5
sanitize-cm-tig5: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_TIG5_CORE) $(CM_TIG5_CHECK) tests/test_cm_tig5.c -o build/test-cm-tig5-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-tig5-sanitize
c64-cm-tig5: build/cm-tig5.prg
build/cm-tig5.prg: $(CM_TIG5_CORE) $(CM_TIG5_CHECK) platform/c64/cm_tig5_diagnostic.c $(CM_TIG5_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-tig5.map $(CM_TIG5_CORE) $(CM_TIG5_CHECK) platform/c64/cm_tig5_diagnostic.c -o $@
