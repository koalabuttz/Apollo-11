# SPDX-License-Identifier: MIT
# Milestone danzig-pending (lane A): the transfer ABI's pending right-hand
# order point at DANZIG after RTB-left order pairs, checked against yaAGC.
DZP_SRC = src/agc_transfer.c src/agc_word.c tests/danzig_pending_check.c
DZP_HDR = include/agc_transfer.h tests/danzig_pending_check.h tests/danzig_pending_vectors.h
.PHONY: test-danzig-pending sanitize-danzig-pending c64-danzig-pending vice-danzig-pending danzig-pending-differential
all: build/test-danzig-pending
test: test-danzig-pending
sanitize: sanitize-danzig-pending
build/test-danzig-pending: $(DZP_SRC) tests/test_danzig_pending.c $(DZP_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DZP_SRC) tests/test_danzig_pending.c -o $@
test-danzig-pending: build/test-danzig-pending
	./build/test-danzig-pending
sanitize-danzig-pending: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(DZP_SRC) tests/test_danzig_pending.c -o build/test-danzig-pending-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-danzig-pending-sanitize
# Host-only oracle run (needs deps/virtualagc and both listing builds).
danzig-pending-differential:
	python3 tools/danzig_pending_freeze.py
	git diff --exit-code -- tests/danzig_pending_vectors.h
c64-danzig-pending: build/danzig-pending.prg
build/danzig-pending.prg: $(DZP_SRC) platform/c64/danzig_pending_diagnostic.c $(DZP_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/danzig-pending.map $(DZP_SRC) platform/c64/danzig_pending_diagnostic.c -o $@
vice-danzig-pending: build/danzig-pending.prg
	python3 tools/vice_run.py build/danzig-pending.prg --cycles 3000000000 --timeout 1200
