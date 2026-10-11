# SPDX-License-Identifier: MIT
DSKY_NV_UPDATE_GENERAL_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_NV_UPDATE_GENERAL_CORE = src/agc_word.c src/dsky_nv_update_general.c src/agc_multiword.c src/dsky_pinball.c src/noun_reader.c src/fixed_data.c tests/dsky_nv_update_general_cases.c tests/dsky_nv_update_general_check.c
all: build/test-dsky-nv-update-general build/dsky-nv-update-general-route-port
build/test-dsky-nv-update-general: tests/test_dsky_nv_update_general.c $(DSKY_NV_UPDATE_GENERAL_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_NV_UPDATE_GENERAL_FLAGS) $^ -o $@
build/dsky-nv-update-general-route-port: tools/dsky_nv_update_general_snapshot.c $(DSKY_NV_UPDATE_GENERAL_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_NV_UPDATE_GENERAL_FLAGS) $^ -o $@
dsky-nv-update-general-host: build/test-dsky-nv-update-general
	build/test-dsky-nv-update-general
dsky-nv-update-general-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(DSKY_NV_UPDATE_GENERAL_FLAGS) tests/test_dsky_nv_update_general.c $(DSKY_NV_UPDATE_GENERAL_CORE) -o build/test-dsky-nv-update-general-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-nv-update-general-sanitize
	python3 tools/dsky_nv_update_general_sanitize.py
dsky-nv-update-general-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(filter-out -Wconversion,$(DSKY_NV_UPDATE_GENERAL_FLAGS)) tools/dsky_nv_update_general_diagnostic.c $(DSKY_NV_UPDATE_GENERAL_CORE) -Wl,-Map,build/dsky_nv_update_general.map -o build/dsky_nv_update_general.prg
test: dsky-nv-update-general-host
sanitize: dsky-nv-update-general-sanitize
.PHONY: dsky-nv-update-general-host dsky-nv-update-general-sanitize dsky-nv-update-general-c64
