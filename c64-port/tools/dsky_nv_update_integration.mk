# SPDX-License-Identifier: MIT
INTEGRATED_DSKY_NV_UPDATE_CORE = src/agc_word.c src/dsky_nv_update.c src/agc_multiword.c src/cm_formatter.c src/dsky_two_digit.c src/dsky_pinball.c src/dsky_decimal.c src/noun_reader.c src/fixed_data.c tests/dsky_nv_update_cases.c tests/dsky_nv_update_check.c
INTEGRATED_DSKY_NV_UPDATE_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
all: build/test-dsky-nv-update-integrated build/dsky-nv-update-integrated-stream
build/test-dsky-nv-update-integrated: tests/test_dsky_nv_update.c $(INTEGRATED_DSKY_NV_UPDATE_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_NV_UPDATE_FLAGS) $^ -o $@
build/dsky-nv-update-integrated-stream: tools/dsky_nv_update_snapshot.c $(INTEGRATED_DSKY_NV_UPDATE_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_NV_UPDATE_FLAGS) $^ -o $@
test: test-dsky-nv-update-integrated
test-dsky-nv-update-integrated: build/test-dsky-nv-update-integrated build/dsky-nv-update-integrated-stream
	./build/test-dsky-nv-update-integrated
	python3 tools/qualified_stream_host.py --archive evidence/dsky-nv-update-reference.snapshots.xz --sha256 f4885c0bf248f0d39b1b2c0a80dea97e9adc2f6104ac107f71b524c61f73eb3f --binary build/dsky-nv-update-integrated-stream
sanitize: sanitize-dsky-nv-update-integrated
sanitize-dsky-nv-update-integrated: | build
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_NV_UPDATE_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_dsky_nv_update.c $(INTEGRATED_DSKY_NV_UPDATE_CORE) -o build/test-dsky-nv-update-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-nv-update-integrated-san
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_NV_UPDATE_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/dsky_nv_update_snapshot.c $(INTEGRATED_DSKY_NV_UPDATE_CORE) -o build/dsky-nv-update-integrated-stream-san
	python3 tools/qualified_stream_host.py --archive evidence/dsky-nv-update-reference.snapshots.xz --sha256 f4885c0bf248f0d39b1b2c0a80dea97e9adc2f6104ac107f71b524c61f73eb3f --binary build/dsky-nv-update-integrated-stream-san
.PHONY: test-dsky-nv-update-integrated sanitize-dsky-nv-update-integrated
