# SPDX-License-Identifier: MIT
INTERPRETER_NATIVE_INPUT = tests/interpreter_native_cases.c
INTERPRETER_NATIVE_CHECK = $(INTERPRETER_NATIVE_INPUT) tests/interpreter_native_check.c tests/state_digest.c
INTERPRETER_NATIVE_HEADERS = include/native_dispatch.h include/executive.h include/irq_delivery.h tests/interpreter_native_cases.h tests/interpreter_native_check.h tests/interpreter_native_vectors.h
all: build/interpreter-native-snapshot build/test-interpreter-native
build/interpreter-native-snapshot: $(CORE) $(INTERPRETER_NATIVE_INPUT) platform/host/interpreter_native_snapshot.c $(INTERPRETER_NATIVE_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CORE) $(INTERPRETER_NATIVE_INPUT) platform/host/interpreter_native_snapshot.c -o $@
build/test-interpreter-native: $(CORE) $(INTERPRETER_NATIVE_CHECK) tests/test_interpreter_native.c $(INTERPRETER_NATIVE_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CORE) $(INTERPRETER_NATIVE_CHECK) tests/test_interpreter_native.c -o $@
.PHONY: test-interpreter-native sanitize-interpreter-native c64-interpreter-native
test: test-interpreter-native
test-interpreter-native: build/test-interpreter-native
	./build/test-interpreter-native
sanitize: sanitize-interpreter-native
sanitize-interpreter-native: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CORE) $(INTERPRETER_NATIVE_CHECK) tests/test_interpreter_native.c -o build/test-interpreter-native-sanitize
	./build/test-interpreter-native-sanitize
c64-interpreter-native: build/interpreter-native.prg
build/interpreter-native.prg: $(CORE) $(INTERPRETER_NATIVE_CHECK) platform/c64/interpreter_native_diagnostic.c platform/c64/irq_platform.c platform/c64/irq_platform.h platform/c64/irq_gate.S $(INTERPRETER_NATIVE_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -Iplatform/c64 -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/interpreter-native.map $(CORE) $(INTERPRETER_NATIVE_CHECK) platform/c64/interpreter_native_diagnostic.c platform/c64/irq_platform.c platform/c64/irq_gate.S -o $@
