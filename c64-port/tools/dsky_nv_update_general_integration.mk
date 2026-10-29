# SPDX-License-Identifier: MIT
INTEGRATED_DSKY_NV_UPDATE_GENERAL_CORE = src/agc_word.c src/dsky_nv_update_general.c src/agc_multiword.c src/dsky_pinball.c src/noun_reader.c src/fixed_data.c tests/dsky_nv_update_general_cases.c tests/dsky_nv_update_general_check.c
INTEGRATED_DSKY_NV_UPDATE_GENERAL_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
all: build/test-dsky-nv-update-general-integrated build/dsky-nv-update-general-integrated-stream
build/test-dsky-nv-update-general-integrated: tests/test_dsky_nv_update_general.c $(INTEGRATED_DSKY_NV_UPDATE_GENERAL_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_NV_UPDATE_GENERAL_FLAGS) $^ -o $@
build/dsky-nv-update-general-integrated-stream: tools/dsky_nv_update_general_snapshot.c $(INTEGRATED_DSKY_NV_UPDATE_GENERAL_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_NV_UPDATE_GENERAL_FLAGS) $^ -o $@
test: test-dsky-nv-update-general-integrated
test-dsky-nv-update-general-integrated: build/test-dsky-nv-update-general-integrated build/dsky-nv-update-general-integrated-stream
	./build/test-dsky-nv-update-general-integrated
	python3 tools/qualified_stream_host.py --archive evidence/dsky-nv-update-general-reference.snapshots.xz --sha256 4de6fafbe18c15bcacc037d5033948ca26522a620a846c76c5224f5955bcdc28 --binary build/dsky-nv-update-general-integrated-stream
sanitize: sanitize-dsky-nv-update-general-integrated
sanitize-dsky-nv-update-general-integrated: | build
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_NV_UPDATE_GENERAL_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_dsky_nv_update_general.c $(INTEGRATED_DSKY_NV_UPDATE_GENERAL_CORE) -o build/test-dsky-nv-update-general-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-nv-update-general-integrated-san
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_NV_UPDATE_GENERAL_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/dsky_nv_update_general_snapshot.c $(INTEGRATED_DSKY_NV_UPDATE_GENERAL_CORE) -o build/dsky-nv-update-general-integrated-stream-san
	python3 tools/qualified_stream_host.py --archive evidence/dsky-nv-update-general-reference.snapshots.xz --sha256 4de6fafbe18c15bcacc037d5033948ca26522a620a846c76c5224f5955bcdc28 --binary build/dsky-nv-update-general-integrated-stream-san
.PHONY: test-dsky-nv-update-general-integrated sanitize-dsky-nv-update-general-integrated
