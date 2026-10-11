# SPDX-License-Identifier: MIT
DSKY_OCTAL_CONSUMER_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_OCTAL_CONSUMER_CORE = src/agc_word.c src/agc_multiword.c src/dsky_pinball.c src/dsky_octal_display.c src/dsky_octal_verb.c src/dsky_octal_consumer.c tests/dsky_octal_verb_cases.c tests/dsky_octal_consumer_cases.c tests/dsky_octal_consumer_check.c
all: build/test-dsky-octal-consumer build/dsky-octal-consumer-route-port
build/test-dsky-octal-consumer: tests/test_dsky_octal_consumer.c $(DSKY_OCTAL_CONSUMER_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_OCTAL_CONSUMER_FLAGS) $^ -o $@
build/dsky-octal-consumer-route-port: tools/dsky_octal_consumer_snapshot.c $(DSKY_OCTAL_CONSUMER_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_OCTAL_CONSUMER_FLAGS) $^ -o $@
dsky-octal-consumer-host: build/test-dsky-octal-consumer
	build/test-dsky-octal-consumer
dsky-octal-consumer-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(DSKY_OCTAL_CONSUMER_FLAGS) tests/test_dsky_octal_consumer.c $(DSKY_OCTAL_CONSUMER_CORE) -o build/test-dsky-octal-consumer-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-octal-consumer-sanitize
	python3 tools/dsky_octal_consumer_sanitize.py
dsky-octal-consumer-c64:
	python3 tools/dsky_octal_consumer_build.py --mos-cc $(MOS_CC)
test: dsky-octal-consumer-host
sanitize: dsky-octal-consumer-sanitize
.PHONY: dsky-octal-consumer-host dsky-octal-consumer-sanitize dsky-octal-consumer-c64
