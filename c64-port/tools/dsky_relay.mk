DSKY_RELAY_CORE = src/agc_word.c src/dsky_relay.c
DSKY_RELAY_TEST = tests/p66_bias_cases.c tests/dsky_relay_cases.c tests/dsky_relay_check.c
.PHONY: test-dsky-relay sanitize-dsky-relay c64-dsky-relay differential-dsky-relay
test-dsky-relay:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_RELAY_CORE) $(DSKY_RELAY_TEST) tests/test_dsky_relay.c -o build/test-dsky-relay
	./build/test-dsky-relay
sanitize-dsky-relay:
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(DSKY_RELAY_CORE) $(DSKY_RELAY_TEST) tests/test_dsky_relay.c -o build/test-dsky-relay-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-relay-san
c64-dsky-relay:
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(DSKY_RELAY_CORE) $(DSKY_RELAY_TEST) tools/dsky_relay_diagnostic.c -Wl,-Map,build/dsky_relay.map -o build/dsky_relay.prg
differential-dsky-relay:
	python3 tools/dsky_relay_differential.py deps/fixture
	python3 tools/freeze_dsky_relay.py
test: test-dsky-relay
sanitize: sanitize-dsky-relay
