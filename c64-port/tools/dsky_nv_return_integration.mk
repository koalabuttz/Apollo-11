# SPDX-License-Identifier: MIT
DSKY_NV_RETURN_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_NV_RETURN_CORE = src/agc_word.c src/dsky_nv_return.c tests/dsky_nv_return_cases.c tests/dsky_nv_return_check.c
all: build/test-dsky-nv-return build/dsky-nv-return-route-port
build/test-dsky-nv-return: tests/test_dsky_nv_return.c $(DSKY_NV_RETURN_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_NV_RETURN_FLAGS) $^ -o $@
build/dsky-nv-return-route-port: tools/dsky_nv_return_snapshot.c $(DSKY_NV_RETURN_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_NV_RETURN_FLAGS) $^ -o $@

test: test-dsky-nv-return-integrated
test-dsky-nv-return-integrated: build/test-dsky-nv-return build/dsky-nv-return-route-port
	./build/test-dsky-nv-return
	python3 tools/qualified_stream_host.py --archive evidence/dsky-nv-return-reference.snapshots.xz --sha256 23af05ed89e8039eef66bd809b8a384556a2dc8b67d0b3c0be5b7aa5f879e944 --binary build/dsky-nv-return-route-port
sanitize: sanitize-dsky-nv-return-integrated
sanitize-dsky-nv-return-integrated: | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -ffunction-sections -Wl,--gc-sections -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_dsky_nv_return.c src/agc_word.c src/dsky_nv_return.c tests/dsky_nv_return_cases.c tests/dsky_nv_return_check.c -o build/test-dsky-nv-return-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-nv-return-integrated-san
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -ffunction-sections -Wl,--gc-sections -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/dsky_nv_return_snapshot.c src/agc_word.c src/dsky_nv_return.c tests/dsky_nv_return_cases.c tests/dsky_nv_return_check.c -o build/dsky-nv-return-integrated-stream-san
	python3 tools/qualified_stream_host.py --archive evidence/dsky-nv-return-reference.snapshots.xz --sha256 23af05ed89e8039eef66bd809b8a384556a2dc8b67d0b3c0be5b7aa5f879e944 --binary build/dsky-nv-return-integrated-stream-san
.PHONY: test-dsky-nv-return-integrated sanitize-dsky-nv-return-integrated
