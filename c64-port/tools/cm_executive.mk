CM_EXEC_CORE = src/agc_word.c src/cm_executive.c
CM_EXEC_INPUT = tests/cm_executive_cases.c
CM_EXEC_TEST = $(CM_EXEC_INPUT) tests/cm_executive_check.c
CM_EXEC_HEADERS = include/cm_executive.h include/agc_word.h include/executive.h tests/cm_executive_cases.h tests/cm_executive_check.h tests/cm_executive_vectors.h
all: build/test-cm-executive build/cm-executive-snapshot
build/cm-executive-snapshot: $(CM_EXEC_CORE) $(CM_EXEC_INPUT) platform/host/cm_executive_snapshot.c $(CM_EXEC_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_EXEC_CORE) $(CM_EXEC_INPUT) platform/host/cm_executive_snapshot.c -o $@
build/test-cm-executive: $(CM_EXEC_CORE) $(CM_EXEC_TEST) tests/test_cm_executive.c $(CM_EXEC_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_EXEC_CORE) $(CM_EXEC_TEST) tests/test_cm_executive.c -o $@
test: test-cm-executive
.PHONY: test-cm-executive sanitize-cm-executive c64-cm-executive cm-differential
cm-differential: build/cm-executive-snapshot
	python3 tools/cm_prepare_reference.py
	python3 tools/cm_executive_differential.py
test-cm-executive: build/test-cm-executive
	./build/test-cm-executive
sanitize: sanitize-cm-executive
sanitize-cm-executive: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_EXEC_CORE) $(CM_EXEC_TEST) tests/test_cm_executive.c -o build/test-cm-executive-sanitize
	./build/test-cm-executive-sanitize
c64-cm-executive: build/cm-executive.prg
build/cm-executive.prg: $(CM_EXEC_CORE) $(CM_EXEC_TEST) platform/c64/cm_executive_diagnostic.c $(CM_EXEC_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-executive.map $(CM_EXEC_CORE) $(CM_EXEC_TEST) platform/c64/cm_executive_diagnostic.c -o $@
