# SPDX-License-Identifier: MIT
# Milestone lm-tjet-law (lane F): Luminary099 TJET_LAW as a native routine of
# the control-transfer ABI.
LM_TJ_SRC = src/lm_tjet_law.c src/agc_fast_ops.c src/agc_fast_window.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/lm_tjet_law_cases.c
LM_TJ_HDR = include/lm_tjet_law.h include/agc_image_ops.h include/agc_fast_ops.h include/agc_fast_window.h include/agc_image_thin.h include/agc_fast_redirect.h include/agc_transfer.h tests/lm_tjet_law_cases.h
.PHONY: test-lm-tjet-law sanitize-lm-tjet-law c64-lm-tjet-law vice-lm-tjet-law lm-tjet-law-differential
all: build/test-lm-tjet-law
test: test-lm-tjet-law
sanitize: sanitize-lm-tjet-law
build/test-lm-tjet-law: $(LM_TJ_SRC) tests/test_lm_tjet_law.c tests/lm_tjet_law_vectors.h $(LM_TJ_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -DLM_TJ_COUNT_PATHS $(LM_TJ_SRC) tests/test_lm_tjet_law.c -o $@
test-lm-tjet-law: build/test-lm-tjet-law
	./build/test-lm-tjet-law
	python3 tools/lm_tjet_law_source.py
sanitize-lm-tjet-law: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -DLM_TJ_COUNT_PATHS -fsanitize=address,undefined $(LM_TJ_SRC) tests/test_lm_tjet_law.c -o build/test-lm-tjet-law-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-tjet-law-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Luminary099 listing build).
build/lm-tjet-law-native: $(LM_TJ_SRC) platform/host/lm_tjet_law_snapshot.c $(LM_TJ_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_TJ_SRC) platform/host/lm_tjet_law_snapshot.c -o $@
lm-tjet-law-differential: build/lm-tjet-law-native
	python3 tools/lm_tjet_law_freeze.py
	git diff --exit-code -- tests/lm_tjet_law_vectors.h tests/lm_tjet_law_c64_vectors.h
c64-lm-tjet-law: build/lm-tjet-law.prg
build/lm-tjet-law.prg: $(LM_TJ_SRC) platform/c64/lm_tjet_law_diagnostic.c tests/lm_tjet_law_c64_vectors.h $(LM_TJ_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-tjet-law.map $(LM_TJ_SRC) platform/c64/lm_tjet_law_diagnostic.c -o $@
vice-lm-tjet-law: build/lm-tjet-law.prg
	python3 tools/vice_run.py build/lm-tjet-law.prg --cycles 3000000000 --timeout 1200
