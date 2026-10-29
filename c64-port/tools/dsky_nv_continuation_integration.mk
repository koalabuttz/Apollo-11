# SPDX-License-Identifier: MIT
INTEGRATED_DSKY_NV_CONTINUATION_CORE = src/agc_word.c src/dsky_nv_continuation.c src/dsky_nv_return.c tests/dsky_nv_continuation_cases.c tests/dsky_nv_continuation_check.c
INTEGRATED_DSKY_NV_CONTINUATION_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
all: build/test-dsky-nv-continuation-integrated build/dsky-nv-continuation-integrated-stream
build/test-dsky-nv-continuation-integrated: tests/test_dsky_nv_continuation.c $(INTEGRATED_DSKY_NV_CONTINUATION_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_NV_CONTINUATION_FLAGS) $^ -o $@
build/dsky-nv-continuation-integrated-stream: tools/dsky_nv_continuation_snapshot.c $(INTEGRATED_DSKY_NV_CONTINUATION_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_NV_CONTINUATION_FLAGS) $^ -o $@
test: test-dsky-nv-continuation-integrated
test-dsky-nv-continuation-integrated: build/test-dsky-nv-continuation-integrated build/dsky-nv-continuation-integrated-stream
	./build/test-dsky-nv-continuation-integrated
	python3 tools/qualified_stream_host.py --archive evidence/dsky-nv-continuation-reference.snapshots.xz --sha256 f06687efb69ed25382e6bf538edc6ee0898e7992437416a876c167695f1063a5 --binary build/dsky-nv-continuation-integrated-stream
sanitize: sanitize-dsky-nv-continuation-integrated
sanitize-dsky-nv-continuation-integrated: | build
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_NV_CONTINUATION_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_dsky_nv_continuation.c $(INTEGRATED_DSKY_NV_CONTINUATION_CORE) -o build/test-dsky-nv-continuation-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-nv-continuation-integrated-san
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_NV_CONTINUATION_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/dsky_nv_continuation_snapshot.c $(INTEGRATED_DSKY_NV_CONTINUATION_CORE) -o build/dsky-nv-continuation-integrated-stream-san
	python3 tools/qualified_stream_host.py --archive evidence/dsky-nv-continuation-reference.snapshots.xz --sha256 f06687efb69ed25382e6bf538edc6ee0898e7992437416a876c167695f1063a5 --binary build/dsky-nv-continuation-integrated-stream-san
.PHONY: test-dsky-nv-continuation-integrated sanitize-dsky-nv-continuation-integrated
