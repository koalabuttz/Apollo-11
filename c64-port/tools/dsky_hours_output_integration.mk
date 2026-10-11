# SPDX-License-Identifier: MIT
INTEGRATED_DSKY_HOURS_OUTPUT_CORE = src/agc_word.c src/agc_multiword.c src/guidance_product.c src/dsky_pinball.c src/dsky_decimal_raw.c src/dsky_read_pair.c src/dsky_hours_output.c tests/dsky_hours_output_cases.c tests/dsky_hours_output_check.c
INTEGRATED_DSKY_HOURS_OUTPUT_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
all: build/test-dsky-hours-output-integrated build/dsky-hours-output-integrated-stream
build/test-dsky-hours-output-integrated: tests/test_dsky_hours_output.c $(INTEGRATED_DSKY_HOURS_OUTPUT_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_HOURS_OUTPUT_FLAGS) $^ -o $@
build/dsky-hours-output-integrated-stream: tools/dsky_hours_output_snapshot.c $(INTEGRATED_DSKY_HOURS_OUTPUT_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_HOURS_OUTPUT_FLAGS) $^ -o $@
test: test-dsky-hours-output-integrated
test-dsky-hours-output-integrated: build/test-dsky-hours-output-integrated build/dsky-hours-output-integrated-stream
	./build/test-dsky-hours-output-integrated
	python3 tools/qualified_stream_host.py --archive evidence/dsky-hours-output-reference.snapshots.xz --sha256 5dcaffa9905b7d2818c5cb02c72d95f682da650dba21030a157abb2d84931870 --binary build/dsky-hours-output-integrated-stream
sanitize: sanitize-dsky-hours-output-integrated
sanitize-dsky-hours-output-integrated: | build
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_HOURS_OUTPUT_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_dsky_hours_output.c $(INTEGRATED_DSKY_HOURS_OUTPUT_CORE) -o build/test-dsky-hours-output-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-hours-output-integrated-san
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_HOURS_OUTPUT_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/dsky_hours_output_snapshot.c $(INTEGRATED_DSKY_HOURS_OUTPUT_CORE) -o build/dsky-hours-output-integrated-stream-san
	python3 tools/qualified_stream_host.py --archive evidence/dsky-hours-output-reference.snapshots.xz --sha256 5dcaffa9905b7d2818c5cb02c72d95f682da650dba21030a157abb2d84931870 --binary build/dsky-hours-output-integrated-stream-san
.PHONY: test-dsky-hours-output-integrated sanitize-dsky-hours-output-integrated
