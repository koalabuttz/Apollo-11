# SPDX-License-Identifier: MIT
INTEGRATED_DSKY_MINUTES_OUTPUT_CORE = src/agc_word.c src/agc_multiword.c src/guidance_product.c src/dsky_pinball.c src/dsky_fraction_pair.c src/dsky_minutes_output.c tests/dsky_minutes_output_cases.c tests/dsky_minutes_output_check.c
INTEGRATED_DSKY_MINUTES_OUTPUT_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
all: build/test-dsky-minutes-output-integrated build/dsky-minutes-output-integrated-stream
build/test-dsky-minutes-output-integrated: tests/test_dsky_minutes_output.c $(INTEGRATED_DSKY_MINUTES_OUTPUT_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_MINUTES_OUTPUT_FLAGS) $^ -o $@
build/dsky-minutes-output-integrated-stream: tools/dsky_minutes_output_snapshot.c $(INTEGRATED_DSKY_MINUTES_OUTPUT_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_MINUTES_OUTPUT_FLAGS) $^ -o $@
test: test-dsky-minutes-output-integrated
test-dsky-minutes-output-integrated: build/test-dsky-minutes-output-integrated build/dsky-minutes-output-integrated-stream
	./build/test-dsky-minutes-output-integrated
	python3 tools/qualified_stream_host.py --archive evidence/dsky-minutes-output-reference.snapshots.xz --sha256 acedec4a9a8fc2e044ea5417d8672bff79c53e88379a2f9930976f1979ca3a76 --binary build/dsky-minutes-output-integrated-stream
sanitize: sanitize-dsky-minutes-output-integrated
sanitize-dsky-minutes-output-integrated: | build
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_MINUTES_OUTPUT_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_dsky_minutes_output.c $(INTEGRATED_DSKY_MINUTES_OUTPUT_CORE) -o build/test-dsky-minutes-output-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-minutes-output-integrated-san
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_MINUTES_OUTPUT_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/dsky_minutes_output_snapshot.c $(INTEGRATED_DSKY_MINUTES_OUTPUT_CORE) -o build/dsky-minutes-output-integrated-stream-san
	python3 tools/qualified_stream_host.py --archive evidence/dsky-minutes-output-reference.snapshots.xz --sha256 acedec4a9a8fc2e044ea5417d8672bff79c53e88379a2f9930976f1979ca3a76 --binary build/dsky-minutes-output-integrated-stream-san
.PHONY: test-dsky-minutes-output-integrated sanitize-dsky-minutes-output-integrated
