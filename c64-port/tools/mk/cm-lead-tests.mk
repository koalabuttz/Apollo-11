# SPDX-License-Identifier: MIT
# Milestone cm-lead-tests (lane A): Comanche055's system-test entry words
# COMPVER, GTSCPSS1 and REDO in the transfer ABI, checked against yaAGC.
CLT_SRC = tests/cm_lead_tests_cases.c tests/cm_lead_tests_check.c src/agc_transfer.c src/standard_lead_routines.c src/interp_entry.c \
	src/standard_lead.c src/standard_lead_interp.c src/standard_lead_wake.c src/lm_basic_wake.c src/executive.c \
	src/cm_basic_wake.c src/cm_native_image.c src/cm_dispatch.c src/cm_executive.c src/agc_word.c
CLT_HDR = include/agc_transfer.h include/standard_lead_routines.h tests/cm_lead_tests_cases.h \
	tests/cm_lead_tests_check.h tests/cm_lead_tests_vectors.h
.PHONY: test-cm-lead-tests sanitize-cm-lead-tests c64-cm-lead-tests vice-cm-lead-tests cm-lead-tests-differential
all: build/test-cm-lead-tests
test: test-cm-lead-tests
sanitize: sanitize-cm-lead-tests
build/test-cm-lead-tests: $(CLT_SRC) tests/test_cm_lead_tests.c $(CLT_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CLT_SRC) tests/test_cm_lead_tests.c -o $@
test-cm-lead-tests: build/test-cm-lead-tests
	./build/test-cm-lead-tests
sanitize-cm-lead-tests: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CLT_SRC) tests/test_cm_lead_tests.c -o build/test-cm-lead-tests-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-lead-tests-sanitize
# Host-only oracle run (needs deps/virtualagc and the Comanche rope build).
cm-lead-tests-differential:
	python3 tools/cm_lead_tests_freeze.py
	git diff --exit-code -- tests/cm_lead_tests_vectors.h
c64-cm-lead-tests: build/cm-lead-tests.prg
build/cm-lead-tests.prg: $(CLT_SRC) platform/c64/cm_lead_tests_diagnostic.c $(CLT_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-lead-tests.map $(CLT_SRC) platform/c64/cm_lead_tests_diagnostic.c -o $@
vice-cm-lead-tests: build/cm-lead-tests.prg
	python3 tools/vice_run.py build/cm-lead-tests.prg --cycles 3000000000 --timeout 1800
