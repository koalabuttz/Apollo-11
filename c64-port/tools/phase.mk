PHASE_CORE = src/phase.c
PHASE_INPUT = tests/phase_cases.c
PHASE_TEST = $(PHASE_INPUT) tests/phase_check.c
PHASE_HEADERS = $(RESTART_HEADERS) include/phase.h tests/phase_cases.h tests/phase_check.h tests/phase_vectors.h
all: build/phase-snapshot build/test-phase
build/phase-snapshot: $(PHASE_CORE) $(PHASE_INPUT) platform/host/phase_snapshot.c $(PHASE_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(PHASE_CORE) $(PHASE_INPUT) platform/host/phase_snapshot.c -o $@
build/test-phase: $(PHASE_CORE) $(PHASE_TEST) tests/test_phase.c $(PHASE_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(PHASE_CORE) $(PHASE_TEST) tests/test_phase.c -o $@
test: test-phase
.PHONY: test-phase sanitize-phase c64-phase
test-phase: build/test-phase
	./build/test-phase
sanitize: sanitize-phase
sanitize-phase: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(PHASE_CORE) $(PHASE_TEST) tests/test_phase.c -o build/test-phase-sanitize
	./build/test-phase-sanitize
c64-phase: build/phase.prg
build/phase.prg: $(PHASE_CORE) $(PHASE_TEST) platform/c64/phase_diagnostic.c $(PHASE_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/phase.map $(PHASE_CORE) $(PHASE_TEST) platform/c64/phase_diagnostic.c -o $@
