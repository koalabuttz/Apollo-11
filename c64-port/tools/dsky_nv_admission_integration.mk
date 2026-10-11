# SPDX-License-Identifier: MIT
DSKY_NV_ADMISSION_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_NV_ADMISSION_CORE = src/agc_word.c src/dsky_nv_admission.c tests/dsky_nv_admission_cases.c tests/dsky_nv_admission_check.c
all: build/test-dsky-nv-admission build/dsky-nv-admission-route-port
build/test-dsky-nv-admission: tests/test_dsky_nv_admission.c $(DSKY_NV_ADMISSION_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_NV_ADMISSION_FLAGS) $^ -o $@
build/dsky-nv-admission-route-port: tools/dsky_nv_admission_snapshot.c $(DSKY_NV_ADMISSION_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_NV_ADMISSION_FLAGS) $^ -o $@

test: test-dsky-nv-admission-integrated
test-dsky-nv-admission-integrated: build/test-dsky-nv-admission build/dsky-nv-admission-route-port
	./build/test-dsky-nv-admission
	python3 tools/qualified_stream_host.py --archive evidence/dsky-nv-admission-reference.snapshots.xz --sha256 6f5e35295a2e05aaf29058e032e0db5830b64f5b77e2e117a9565f5e899b0eee --binary build/dsky-nv-admission-route-port
sanitize: sanitize-dsky-nv-admission-integrated
sanitize-dsky-nv-admission-integrated: | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -ffunction-sections -Wl,--gc-sections -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_dsky_nv_admission.c src/agc_word.c src/dsky_nv_admission.c tests/dsky_nv_admission_cases.c tests/dsky_nv_admission_check.c -o build/test-dsky-nv-admission-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-nv-admission-integrated-san
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -ffunction-sections -Wl,--gc-sections -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/dsky_nv_admission_snapshot.c src/agc_word.c src/dsky_nv_admission.c tests/dsky_nv_admission_cases.c tests/dsky_nv_admission_check.c -o build/dsky-nv-admission-integrated-stream-san
	python3 tools/qualified_stream_host.py --archive evidence/dsky-nv-admission-reference.snapshots.xz --sha256 6f5e35295a2e05aaf29058e032e0db5830b64f5b77e2e117a9565f5e899b0eee --binary build/dsky-nv-admission-integrated-stream-san
.PHONY: test-dsky-nv-admission-integrated sanitize-dsky-nv-admission-integrated
