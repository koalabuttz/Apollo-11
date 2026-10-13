# SPDX-License-Identifier: MIT
# Milestone interbank-q (lane A): the INTER-BANK entries reached with an
# unusual Q (SWCALL -1 from RTB, BANKJUMP from SAVELOCS' TC L), both missions,
# checked against yaAGC.
IBQ_SRC = tests/interbank_q_cases.c tests/interbank_q_check.c src/lm_interbank_routines.c src/lm_interbank.c \
	src/cm_interbank_routines.c src/cm_interbank_complete.c src/agc_transfer.c src/agc_word.c \
	src/agc_multiword.c src/agc_image_ops.c src/agc_hardware_divide.c
IBQ_HDR = include/agc_transfer.h include/lm_interbank_routines.h include/cm_interbank_routines.h \
	tests/interbank_q_cases.h tests/interbank_q_check.h tests/interbank_q_vectors.h
.PHONY: test-interbank-q sanitize-interbank-q c64-interbank-q vice-interbank-q interbank-q-differential
all: build/test-interbank-q
test: test-interbank-q
sanitize: sanitize-interbank-q
build/test-interbank-q: $(IBQ_SRC) tests/test_interbank_q.c $(IBQ_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(IBQ_SRC) tests/test_interbank_q.c -o $@
test-interbank-q: build/test-interbank-q
	./build/test-interbank-q
sanitize-interbank-q: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(IBQ_SRC) tests/test_interbank_q.c -o build/test-interbank-q-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-interbank-q-sanitize
# Host-only oracle run (needs deps/virtualagc and both rope builds).
interbank-q-differential:
	python3 tools/interbank_q_freeze.py
	git diff --exit-code -- tests/interbank_q_vectors.h
c64-interbank-q: build/interbank-q.prg
build/interbank-q.prg: $(IBQ_SRC) platform/c64/interbank_q_diagnostic.c $(IBQ_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/interbank-q.map $(IBQ_SRC) platform/c64/interbank_q_diagnostic.c -o $@
vice-interbank-q: build/interbank-q.prg
	python3 tools/vice_run.py build/interbank-q.prg --cycles 3000000000 --timeout 1800
