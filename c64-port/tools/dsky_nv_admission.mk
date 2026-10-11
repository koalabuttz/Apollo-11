# SPDX-License-Identifier: MIT
DSKY_NV_ADMISSION_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_NV_ADMISSION_CORE = src/agc_word.c src/dsky_nv_admission.c tests/dsky_nv_admission_cases.c tests/dsky_nv_admission_check.c
all: build/test-dsky-nv-admission build/dsky-nv-admission-route-port
build/test-dsky-nv-admission: tests/test_dsky_nv_admission.c $(DSKY_NV_ADMISSION_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_NV_ADMISSION_FLAGS) $^ -o $@
build/dsky-nv-admission-route-port: tools/dsky_nv_admission_snapshot.c $(DSKY_NV_ADMISSION_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_NV_ADMISSION_FLAGS) $^ -o $@
dsky-nv-admission-host: build/test-dsky-nv-admission
	build/test-dsky-nv-admission
dsky-nv-admission-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(DSKY_NV_ADMISSION_FLAGS) tests/test_dsky_nv_admission.c $(DSKY_NV_ADMISSION_CORE) -o build/test-dsky-nv-admission-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-nv-admission-sanitize
	python3 tools/dsky_nv_admission_sanitize.py
dsky-nv-admission-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_NV_ADMISSION_FLAGS) tools/dsky_nv_admission_diagnostic.c $(DSKY_NV_ADMISSION_CORE) -Wl,-Map,build/dsky_nv_admission.map -o build/dsky_nv_admission.prg
test: dsky-nv-admission-host
sanitize: dsky-nv-admission-sanitize
.PHONY: dsky-nv-admission-host dsky-nv-admission-sanitize dsky-nv-admission-c64
