CM_DISPATCH_CORE = src/agc_word.c src/cm_executive.c src/cm_dispatch.c src/cm_native_dispatch.c
CM_DISPATCH_INPUT = tests/cm_executive_cases.c tests/cm_dispatch_cases.c tests/cm_lifecycle_cases.c tests/cm_interpreter_cases.c tests/cm_mixed_cases.c
CM_DISPATCH_HEADERS = include/cm_executive.h include/cm_dispatch.h include/cm_native_dispatch.h tests/cm_dispatch_cases.h tests/cm_lifecycle_cases.h tests/cm_interpreter_cases.h tests/cm_mixed_cases.h
all: build/cm-dispatch-snapshot build/cm-lifecycle-snapshot build/cm-interpreter-snapshot build/cm-mixed-snapshot build/test-cm-dispatch
build/cm-dispatch-snapshot: $(CM_DISPATCH_CORE) $(CM_DISPATCH_INPUT) platform/host/cm_dispatch_snapshot.c $(CM_DISPATCH_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_DISPATCH_CORE) $(CM_DISPATCH_INPUT) platform/host/cm_dispatch_snapshot.c -o $@
build/cm-lifecycle-snapshot: $(CM_DISPATCH_CORE) $(CM_DISPATCH_INPUT) platform/host/cm_lifecycle_snapshot.c $(CM_DISPATCH_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_DISPATCH_CORE) $(CM_DISPATCH_INPUT) platform/host/cm_lifecycle_snapshot.c -o $@
build/cm-interpreter-snapshot: $(CM_DISPATCH_CORE) $(CM_DISPATCH_INPUT) platform/host/cm_interpreter_snapshot.c $(CM_DISPATCH_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_DISPATCH_CORE) $(CM_DISPATCH_INPUT) platform/host/cm_interpreter_snapshot.c -o $@
build/cm-mixed-snapshot: $(CM_DISPATCH_CORE) $(CM_DISPATCH_INPUT) platform/host/cm_mixed_snapshot.c $(CM_DISPATCH_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_DISPATCH_CORE) $(CM_DISPATCH_INPUT) platform/host/cm_mixed_snapshot.c -o $@
test: test-cm-dispatch
.PHONY: test-cm-dispatch sanitize-cm-dispatch c64-cm-dispatch cm-dispatch-differential
cm-dispatch-differential: build/cm-dispatch-snapshot build/cm-lifecycle-snapshot build/cm-interpreter-snapshot build/cm-mixed-snapshot
	python3 tools/cm_dispatch_differential.py
build/test-cm-dispatch: $(CM_DISPATCH_CORE) $(CM_DISPATCH_INPUT) tests/cm_dispatch_check.c tests/test_cm_dispatch.c $(CM_DISPATCH_HEADERS) tests/cm_dispatch_vectors.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_DISPATCH_CORE) $(CM_DISPATCH_INPUT) tests/cm_dispatch_check.c tests/test_cm_dispatch.c -o $@
test-cm-dispatch: build/test-cm-dispatch
	./build/test-cm-dispatch
sanitize: sanitize-cm-dispatch
sanitize-cm-dispatch: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_DISPATCH_CORE) $(CM_DISPATCH_INPUT) tests/cm_dispatch_check.c tests/test_cm_dispatch.c -o build/test-cm-dispatch-sanitize
	./build/test-cm-dispatch-sanitize
c64-cm-dispatch: build/cm-dispatch-basic.prg build/cm-dispatch-lifecycle.prg build/cm-dispatch-interpreter.prg build/cm-dispatch-mixed.prg
build/cm-dispatch-basic.prg: $(CM_DISPATCH_CORE) $(CM_DISPATCH_INPUT) tests/cm_dispatch_check.c platform/c64/cm_dispatch_diagnostic.c $(CM_DISPATCH_HEADERS) tests/cm_dispatch_vectors.h | build
	$(MOS_CC) $(CPPFLAGS) -DCM_CHECK_BASIC -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-dispatch-basic.map $(CM_DISPATCH_CORE) $(CM_DISPATCH_INPUT) tests/cm_dispatch_check.c platform/c64/cm_dispatch_diagnostic.c -o $@
build/cm-dispatch-lifecycle.prg: $(CM_DISPATCH_CORE) $(CM_DISPATCH_INPUT) tests/cm_dispatch_check.c platform/c64/cm_dispatch_diagnostic.c $(CM_DISPATCH_HEADERS) tests/cm_dispatch_vectors.h | build
	$(MOS_CC) $(CPPFLAGS) -DCM_CHECK_LIFECYCLE -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-dispatch-lifecycle.map $(CM_DISPATCH_CORE) $(CM_DISPATCH_INPUT) tests/cm_dispatch_check.c platform/c64/cm_dispatch_diagnostic.c -o $@
build/cm-dispatch-interpreter.prg: $(CM_DISPATCH_CORE) $(CM_DISPATCH_INPUT) tests/cm_dispatch_check.c platform/c64/cm_dispatch_diagnostic.c $(CM_DISPATCH_HEADERS) tests/cm_dispatch_vectors.h | build
	$(MOS_CC) $(CPPFLAGS) -DCM_CHECK_INTERPRETER -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-dispatch-interpreter.map $(CM_DISPATCH_CORE) $(CM_DISPATCH_INPUT) tests/cm_dispatch_check.c platform/c64/cm_dispatch_diagnostic.c -o $@
build/cm-dispatch-mixed.prg: $(CM_DISPATCH_CORE) $(CM_DISPATCH_INPUT) tests/cm_dispatch_check.c platform/c64/cm_dispatch_diagnostic.c $(CM_DISPATCH_HEADERS) tests/cm_dispatch_vectors.h | build
	$(MOS_CC) $(CPPFLAGS) -DCM_CHECK_MIXED -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-dispatch-mixed.map $(CM_DISPATCH_CORE) $(CM_DISPATCH_INPUT) tests/cm_dispatch_check.c platform/c64/cm_dispatch_diagnostic.c -o $@
