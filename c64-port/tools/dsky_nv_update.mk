# SPDX-License-Identifier: MIT
DSKY_NV_UPDATE_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_NV_UPDATE_CORE = src/agc_word.c src/dsky_nv_update.c src/agc_multiword.c src/cm_formatter.c src/dsky_two_digit.c src/dsky_pinball.c src/dsky_decimal.c src/noun_reader.c src/fixed_data.c tests/dsky_nv_update_cases.c tests/dsky_nv_update_check.c
all: build/test-dsky-nv-update build/dsky-nv-update-route-port
build/test-dsky-nv-update: tests/test_dsky_nv_update.c $(DSKY_NV_UPDATE_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_NV_UPDATE_FLAGS) $^ -o $@
build/dsky-nv-update-route-port: tools/dsky_nv_update_snapshot.c $(DSKY_NV_UPDATE_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_NV_UPDATE_FLAGS) $^ -o $@
dsky-nv-update-host: build/test-dsky-nv-update
	build/test-dsky-nv-update
dsky-nv-update-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(DSKY_NV_UPDATE_FLAGS) tests/test_dsky_nv_update.c $(DSKY_NV_UPDATE_CORE) -o build/test-dsky-nv-update-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-nv-update-sanitize
	python3 tools/dsky_nv_update_sanitize.py
dsky-nv-update-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(filter-out -Wconversion,$(DSKY_NV_UPDATE_FLAGS)) tools/dsky_nv_update_diagnostic.c $(DSKY_NV_UPDATE_CORE) -Wl,-Map,build/dsky_nv_update.map -o build/dsky_nv_update.prg
test: dsky-nv-update-host
sanitize: dsky-nv-update-sanitize
.PHONY: dsky-nv-update-host dsky-nv-update-sanitize dsky-nv-update-c64
