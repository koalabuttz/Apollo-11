CM_PHASE_CORE = src/cm_phase.c
CM_PHASE_INPUT = tests/cm_phase_cases.c
CM_PHASE_TEST = $(CM_PHASE_INPUT) tests/cm_phase_check.c
CM_PHASE_HEADERS = include/cm_restart.h include/cm_executive.h include/cm_waitlist.h include/cm_phase.h tests/cm_phase_cases.h tests/cm_phase_check.h tests/cm_phase_vectors.h
all: build/cm-phase-snapshot build/test-cm-phase
build/cm-phase-snapshot: $(CM_PHASE_CORE) $(CM_PHASE_INPUT) platform/host/cm_phase_snapshot.c $(CM_PHASE_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_PHASE_CORE) $(CM_PHASE_INPUT) platform/host/cm_phase_snapshot.c -o $@
build/test-cm-phase: $(CM_PHASE_CORE) $(CM_PHASE_TEST) tests/test_cm_phase.c $(CM_PHASE_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_PHASE_CORE) $(CM_PHASE_TEST) tests/test_cm_phase.c -o $@
test: test-cm-phase
.PHONY: test-cm-phase sanitize-cm-phase c64-cm-phase
test-cm-phase: build/test-cm-phase
	./build/test-cm-phase
sanitize: sanitize-cm-phase
sanitize-cm-phase: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_PHASE_CORE) $(CM_PHASE_TEST) tests/test_cm_phase.c -o build/test-cm-phase-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-phase-sanitize
c64-cm-phase: build/cm-phase.prg
build/cm-phase.prg: $(CM_PHASE_CORE) $(CM_PHASE_TEST) platform/c64/cm_phase_diagnostic.c $(CM_PHASE_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-phase.map $(CM_PHASE_CORE) $(CM_PHASE_TEST) platform/c64/cm_phase_diagnostic.c -o $@
.PHONY: cm-phase-differential
cm-phase-differential: build/cm-phase-snapshot
	python3 tools/cm_phase_differential.py
