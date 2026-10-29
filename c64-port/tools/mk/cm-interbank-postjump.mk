# SPDX-License-Identifier: MIT
# The CM self-check idle loop's POSTJUMP table (lane D2, docs/FLIGHT-STORE.md):
# src/cm_interbank_postjump.c against cm_interbank_routines on random states.
CM_IBP_SRC = src/cm_interbank_postjump.c src/cm_interbank_routines.c src/cm_interbank_idle.c src/cm_interbank_complete.c src/agc_transfer.c src/agc_word.c
CM_IBP_HDR = include/cm_interbank_postjump.h include/cm_interbank_routines.h include/cm_interbank_complete.h include/agc_transfer.h
.PHONY: test-cm-interbank-postjump sanitize-cm-interbank-postjump
all: build/test-cm-interbank-postjump
test: test-cm-interbank-postjump
sanitize: sanitize-cm-interbank-postjump
build/test-cm-interbank-postjump: $(CM_IBP_SRC) tests/test_cm_interbank_postjump.c $(CM_IBP_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_IBP_SRC) tests/test_cm_interbank_postjump.c -o $@
test-cm-interbank-postjump: build/test-cm-interbank-postjump
	./build/test-cm-interbank-postjump
sanitize-cm-interbank-postjump: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_IBP_SRC) tests/test_cm_interbank_postjump.c -o build/test-cm-interbank-postjump-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-interbank-postjump-sanitize
