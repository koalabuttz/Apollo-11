# SPDX-License-Identifier: MIT
SERVICE_INTEGRATED_CORE = src/service_routines.c src/agc_word.c tests/service_routines_cases.c
SERVICE_INTEGRATED_TEST = tests/test_service_routines.c tests/service_routines_target.c tests/frozen_words.c
all: build/service-routines-integrated-stream build/test-service-routines-integrated
build/service-routines-integrated-stream: tools/service_routines_snapshot.c $(SERVICE_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/test-service-routines-integrated: $(SERVICE_INTEGRATED_TEST) $(SERVICE_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test: test-service-routines-integrated
test-service-routines-integrated: build/service-routines-integrated-stream build/test-service-routines-integrated
	./build/test-service-routines-integrated
	python3 tools/qualified_stream_host.py --binary build/service-routines-integrated-stream --archive evidence/service-routines-reference.snapshots.bin.xz --sha256 860235bc19064c6d945c0afff223ce2e9cc9e5a2cfa4df273cdb6fc010877a8f --bytes 437485568
sanitize: sanitize-service-routines-integrated
sanitize-service-routines-integrated: | build
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(SERVICE_INTEGRATED_TEST) $(SERVICE_INTEGRATED_CORE) -o build/test-service-routines-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-service-routines-integrated-san
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/service_routines_snapshot.c $(SERVICE_INTEGRATED_CORE) -o build/service-routines-integrated-stream-san
	python3 tools/qualified_stream_host.py --binary build/service-routines-integrated-stream-san --archive evidence/service-routines-reference.snapshots.bin.xz --sha256 860235bc19064c6d945c0afff223ce2e9cc9e5a2cfa4df273cdb6fc010877a8f --bytes 437485568
.PHONY: test-service-routines-integrated sanitize-service-routines-integrated
