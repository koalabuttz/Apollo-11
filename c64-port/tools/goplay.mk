# SPDX-License-Identifier: MIT
GPLAY_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
GPLAY_CORE = src/agc_word.c src/p66_goplay.c tests/throttle_bridge_cases.c tests/goplay_cases.c
GPLAY_FULL_CORE = src/p66_nv_render.c src/dsky_scale.c src/guidance_product.c src/p66_nv_words.c src/p66_nv_rows.c src/p66_nv_entry.c src/dsky_two_digit.c src/dsky_pinball.c src/dsky_decimal.c src/agc_multiword.c $(GPLAY_CORE)
goplay-host:
	$(CC) -O2 -Wconversion -pedantic $(GPLAY_FLAGS) tests/test_goplay.c tests/goplay_check.c $(GPLAY_CORE) -o build/test-goplay
	build/test-goplay
goplay-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(GPLAY_FLAGS) tests/test_goplay.c tests/goplay_check.c $(GPLAY_CORE) -o build/test-goplay-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-goplay-sanitize
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(GPLAY_FLAGS) tools/goplay_snapshot.c $(GPLAY_CORE) -o build/goplay-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/goplay-native-sanitize > build/goplay-sanitize.snapshots
	cmp build/goplay-sanitize.snapshots build/goplay-reference.snapshots
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(GPLAY_FLAGS) tools/goplay_full_snapshot.c $(GPLAY_FULL_CORE) -o build/goplay-full-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/goplay-full-native-sanitize > build/goplay-full-sanitize.snapshots
	cmp build/goplay-full-sanitize.snapshots build/goplay-full-reference.snapshots
goplay-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(GPLAY_FLAGS) tools/goplay_diagnostic.c tests/goplay_check.c $(GPLAY_CORE) -Wl,-Map,build/goplay.map -o build/goplay.prg
goplay-differential:
	python3 tools/goplay_differential.py
	python3 tools/goplay_full_differential.py
all: build/goplay-native build/test-goplay
build/goplay-native: tools/goplay_snapshot.c $(GPLAY_CORE) | build
	$(CC) -O2 $(GPLAY_FLAGS) tools/goplay_snapshot.c $(GPLAY_CORE) -o $@
build/test-goplay: tests/test_goplay.c tests/goplay_check.c tests/goplay_vectors.h $(GPLAY_CORE) | build
	$(CC) -O2 -Wconversion -pedantic $(GPLAY_FLAGS) tests/test_goplay.c tests/goplay_check.c $(GPLAY_CORE) -o $@
test: goplay-host
sanitize: goplay-sanitize
.PHONY: goplay-host goplay-sanitize goplay-c64 goplay-differential
