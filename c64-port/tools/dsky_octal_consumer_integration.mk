# SPDX-License-Identifier: MIT
INTEGRATED_DSKY_OCTAL_CONSUMER_CORE = src/agc_word.c src/agc_multiword.c src/dsky_pinball.c src/dsky_octal_display.c src/dsky_octal_verb.c src/dsky_octal_consumer.c tests/dsky_octal_verb_cases.c tests/dsky_octal_consumer_cases.c tests/dsky_octal_consumer_check.c
INTEGRATED_DSKY_OCTAL_CONSUMER_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
all: build/test-dsky-octal-consumer-integrated build/dsky-octal-consumer-integrated-stream
build/test-dsky-octal-consumer-integrated: tests/test_dsky_octal_consumer.c $(INTEGRATED_DSKY_OCTAL_CONSUMER_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_OCTAL_CONSUMER_FLAGS) $^ -o $@
build/dsky-octal-consumer-integrated-stream: tools/dsky_octal_consumer_snapshot.c $(INTEGRATED_DSKY_OCTAL_CONSUMER_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_OCTAL_CONSUMER_FLAGS) $^ -o $@
test: test-dsky-octal-consumer-integrated
test-dsky-octal-consumer-integrated: build/test-dsky-octal-consumer-integrated build/dsky-octal-consumer-integrated-stream
	./build/test-dsky-octal-consumer-integrated
	python3 tools/qualified_stream_host.py --archive evidence/dsky-octal-consumer-reference.snapshots.xz --sha256 46f199fe0f15f16f92097ab16a7aee2efc682af9f5297259d9215a43e66066ac --binary build/dsky-octal-consumer-integrated-stream
sanitize: sanitize-dsky-octal-consumer-integrated
sanitize-dsky-octal-consumer-integrated: | build
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_OCTAL_CONSUMER_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_dsky_octal_consumer.c $(INTEGRATED_DSKY_OCTAL_CONSUMER_CORE) -o build/test-dsky-octal-consumer-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-octal-consumer-integrated-san
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_OCTAL_CONSUMER_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/dsky_octal_consumer_snapshot.c $(INTEGRATED_DSKY_OCTAL_CONSUMER_CORE) -o build/dsky-octal-consumer-integrated-stream-san
	python3 tools/qualified_stream_host.py --archive evidence/dsky-octal-consumer-reference.snapshots.xz --sha256 46f199fe0f15f16f92097ab16a7aee2efc682af9f5297259d9215a43e66066ac --binary build/dsky-octal-consumer-integrated-stream-san
.PHONY: test-dsky-octal-consumer-integrated sanitize-dsky-octal-consumer-integrated
