# SPDX-License-Identifier: MIT
SBAND_SHIFT_INTEGRATED_CORE = tests/sband_shift_cases.c src/agc_word.c src/sband_shift.c src/agc_multiword.c src/guidance_shift.c
SBAND_SHIFT_INTEGRATED_TEST = tests/test_sband_shift.c tests/sband_shift_target.c tests/frozen_words.c
build/sband-shift-integrated-stream: tools/sband_shift_snapshot.c $(SBAND_SHIFT_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/test-sband-shift-integrated: $(SBAND_SHIFT_INTEGRATED_TEST) $(SBAND_SHIFT_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test-sband-shift: build/sband-shift-integrated-stream build/test-sband-shift-integrated
	./build/test-sband-shift-integrated
	python3 tools/sband_shift_port_host.py --binary build/sband-shift-integrated-stream
sanitize-sband-shift: | build
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(SBAND_SHIFT_INTEGRATED_TEST) $(SBAND_SHIFT_INTEGRATED_CORE) -o build/test-sband-shift-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-sband-shift-integrated-san
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/sband_shift_snapshot.c $(SBAND_SHIFT_INTEGRATED_CORE) -o build/sband-shift-integrated-stream-san
	python3 tools/sband_shift_port_host.py --binary build/sband-shift-integrated-stream-san
.PHONY: test-sband-shift sanitize-sband-shift
