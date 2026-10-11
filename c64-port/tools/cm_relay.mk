# SPDX-License-Identifier: MIT
CM_RELAY_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
CM_RELAY_CORE = src/agc_word.c src/dsky_relay.c src/dsky_display.c src/cm_relay.c tests/cm_pinball_cases.c tests/cm_relay_cases.c
CM_RELAY_TEST = tests/test_cm_relay.c tests/cm_relay_check.c
all: build/test-cm-relay
build/test-cm-relay: $(CM_RELAY_TEST) $(CM_RELAY_CORE) tests/cm_relay_vectors.h
	@mkdir -p build
	$(CC) -O2 -Wconversion -pedantic $(CM_RELAY_FLAGS) $(CM_RELAY_TEST) $(CM_RELAY_CORE) -o $@
test-cm-relay: build/test-cm-relay
	build/test-cm-relay
sanitize-cm-relay:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CM_RELAY_FLAGS) $(CM_RELAY_TEST) $(CM_RELAY_CORE) -o build/test-cm-relay-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-relay-sanitize
c64-cm-relay:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(CM_RELAY_FLAGS) tools/cm_relay_diagnostic.c tests/cm_relay_check.c $(CM_RELAY_CORE) -Wl,-Map,build/cm_relay.map -o build/cm_relay.prg
differential-cm-relay:
	python3 tools/cm_relay_differential.py
test: test-cm-relay
sanitize: sanitize-cm-relay
.PHONY: test-cm-relay sanitize-cm-relay c64-cm-relay differential-cm-relay
