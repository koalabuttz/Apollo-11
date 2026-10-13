# SPDX-License-Identifier: MIT
# Milestone interp-entry (lane A): INTERPRETER.agc's INTPRET, INTPRET +2 and
# INTRSM of Luminary099 and Comanche055 for any caller, as routines of
# lm_interpreter_routines and cm_interpreter_routines ending at NEWOPS.
IE_SRC = src/interp_entry.c src/agc_transfer.c src/standard_lead_routines.c src/standard_lead.c src/standard_lead_interp.c src/standard_lead_wake.c src/lm_basic_wake.c src/executive.c src/cm_basic_wake.c src/cm_native_image.c src/cm_dispatch.c src/cm_executive.c src/agc_word.c tests/interp_entry_cases.c
IE_HDR = include/interp_entry.h include/standard_lead_routines.h include/agc_transfer.h tests/interp_entry_cases.h
IE_FLAGS = -ffunction-sections -fdata-sections -Wl,--gc-sections
.PHONY: test-interp-entry sanitize-interp-entry c64-interp-entry vice-interp-entry interp-entry-differential
all: build/test-interp-entry
test: test-interp-entry
sanitize: sanitize-interp-entry
build/test-interp-entry: $(IE_SRC) tests/test_interp_entry.c tests/interp_entry_vectors.h $(IE_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(IE_FLAGS) $(IE_SRC) tests/test_interp_entry.c -o $@
test-interp-entry: build/test-interp-entry
	./build/test-interp-entry
sanitize-interp-entry: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(IE_FLAGS) -fsanitize=address,undefined $(IE_SRC) tests/test_interp_entry.c -o build/test-interp-entry-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-interp-entry-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
build/interp-entry-native: $(IE_SRC) platform/host/interp_entry_snapshot.c $(IE_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(IE_FLAGS) $(IE_SRC) platform/host/interp_entry_snapshot.c -o $@
interp-entry-differential: build/interp-entry-native
	python3 tools/interp_entry_freeze.py
	git diff --exit-code -- tests/interp_entry_vectors.h tests/interp_entry_c64_vectors.h
c64-interp-entry: build/interp-entry.prg
build/interp-entry.prg: $(IE_SRC) platform/c64/interp_entry_diagnostic.c tests/interp_entry_c64_vectors.h $(IE_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(IE_FLAGS) -Wl,-Map,build/interp-entry.map $(IE_SRC) platform/c64/interp_entry_diagnostic.c -o $@
vice-interp-entry: build/interp-entry.prg
	python3 tools/vice_run.py build/interp-entry.prg --cycles 3000000000 --timeout 1200
