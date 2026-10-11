# SPDX-License-Identifier: MIT
INTEGRATED_DSKY_NV_BLANK_CORE = src/agc_word.c src/dsky_nv_blank.c tests/dsky_nv_blank_cases.c tests/dsky_nv_blank_check.c
INTEGRATED_DSKY_NV_BLANK_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
all: build/test-dsky-nv-blank-integrated build/dsky-nv-blank-integrated-stream
build/test-dsky-nv-blank-integrated: tests/test_dsky_nv_blank.c $(INTEGRATED_DSKY_NV_BLANK_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_NV_BLANK_FLAGS) $^ -o $@
build/dsky-nv-blank-integrated-stream: tools/dsky_nv_blank_snapshot.c $(INTEGRATED_DSKY_NV_BLANK_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_NV_BLANK_FLAGS) $^ -o $@
test: test-dsky-nv-blank-integrated
test-dsky-nv-blank-integrated: build/test-dsky-nv-blank-integrated build/dsky-nv-blank-integrated-stream
	./build/test-dsky-nv-blank-integrated
	python3 tools/qualified_stream_host.py --archive evidence/dsky-nv-blank-reference.snapshots.xz --sha256 2bc6ef4cec4c58d850eaa9ea91bde4a8050eb9e42b4e5e1efb796382a1e88dbc --binary build/dsky-nv-blank-integrated-stream
sanitize: sanitize-dsky-nv-blank-integrated
sanitize-dsky-nv-blank-integrated: | build
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_NV_BLANK_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_dsky_nv_blank.c $(INTEGRATED_DSKY_NV_BLANK_CORE) -o build/test-dsky-nv-blank-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-nv-blank-integrated-san
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_NV_BLANK_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/dsky_nv_blank_snapshot.c $(INTEGRATED_DSKY_NV_BLANK_CORE) -o build/dsky-nv-blank-integrated-stream-san
	python3 tools/qualified_stream_host.py --archive evidence/dsky-nv-blank-reference.snapshots.xz --sha256 2bc6ef4cec4c58d850eaa9ea91bde4a8050eb9e42b4e5e1efb796382a1e88dbc --binary build/dsky-nv-blank-integrated-stream-san
.PHONY: test-dsky-nv-blank-integrated sanitize-dsky-nv-blank-integrated
