# SPDX-License-Identifier: MIT
CM_INTERBANK_COMPLETE_INTEGRATION_CORE = tests/cm_interbank_complete_target.c tests/cm_interbank_complete_cases.c tests/frozen_words.c src/cm_interbank_complete.c src/agc_word.c
all: build/test-cm-interbank-complete-integrated
build/test-cm-interbank-complete-integrated: tests/test_cm_interbank_complete.c $(CM_INTERBANK_COMPLETE_INTEGRATION_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffunction-sections -Wl,--gc-sections $^ -o $@
test: test-cm-interbank-complete-integrated
test-cm-interbank-complete-integrated: build/test-cm-interbank-complete-integrated
	./build/test-cm-interbank-complete-integrated
sanitize: sanitize-cm-interbank-complete-integrated
sanitize-cm-interbank-complete-integrated: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -ffunction-sections -Wl,--gc-sections -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_cm_interbank_complete.c $(CM_INTERBANK_COMPLETE_INTEGRATION_CORE) -o build/test-cm-interbank-complete-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-interbank-complete-integrated-san
.PHONY: test-cm-interbank-complete-integrated sanitize-cm-interbank-complete-integrated
