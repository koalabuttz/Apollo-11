# SPDX-License-Identifier: MIT
INTEGRATED_DSKY_FIXED_DISPLAY_CORE = src/agc_word.c src/dsky_fixed_display.c src/lm_interbank.c src/cm_fixed_callers.c src/dsky_octal_display.c src/dsky_pinball.c tests/dsky_fixed_display_cases.c
INTEGRATED_DSKY_FIXED_DISPLAY_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
all: build/test-dsky-fixed-display-integrated build/dsky-fixed-display-integrated-stream
build/test-dsky-fixed-display-integrated: tests/test_dsky_fixed_display.c tests/dsky_fixed_display_check.c $(INTEGRATED_DSKY_FIXED_DISPLAY_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_FIXED_DISPLAY_FLAGS) $^ -o $@
build/dsky-fixed-display-integrated-stream: tools/dsky_fixed_display_snapshot.c src/dsky_fixed_display_data.c src/lm_fixed_compat.c src/cm_fixed_compat.c $(INTEGRATED_DSKY_FIXED_DISPLAY_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_FIXED_DISPLAY_FLAGS) $^ -o $@
test: test-dsky-fixed-display-integrated
test-dsky-fixed-display-integrated: build/test-dsky-fixed-display-integrated build/dsky-fixed-display-integrated-stream
	./build/test-dsky-fixed-display-integrated
	python3 tools/qualified_stream_host.py --archive evidence/dsky-fixed-display-reference.snapshots.xz --sha256 918f49396ad4af3cee4de5f48a038a7c3f385afc2b3707ceb11db4fd724d8133 --binary build/dsky-fixed-display-integrated-stream
sanitize: sanitize-dsky-fixed-display-integrated
sanitize-dsky-fixed-display-integrated: | build
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_FIXED_DISPLAY_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_dsky_fixed_display.c tests/dsky_fixed_display_check.c $(INTEGRATED_DSKY_FIXED_DISPLAY_CORE) -o build/test-dsky-fixed-display-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-fixed-display-integrated-san
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_FIXED_DISPLAY_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/dsky_fixed_display_snapshot.c src/dsky_fixed_display_data.c src/lm_fixed_compat.c src/cm_fixed_compat.c $(INTEGRATED_DSKY_FIXED_DISPLAY_CORE) -o build/dsky-fixed-display-integrated-stream-san
	python3 tools/qualified_stream_host.py --archive evidence/dsky-fixed-display-reference.snapshots.xz --sha256 918f49396ad4af3cee4de5f48a038a7c3f385afc2b3707ceb11db4fd724d8133 --binary build/dsky-fixed-display-integrated-stream-san
.PHONY: test-dsky-fixed-display-integrated sanitize-dsky-fixed-display-integrated
