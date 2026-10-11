P66_NATIVE_CORE = $(VELOCITY_CORE) src/executive.c src/native_dispatch.c src/irq_delivery.c src/p66_native.c
P66_NATIVE_INPUT = $(VELOCITY_INPUT) tests/p66_native_cases.c
P66_NATIVE_HEADERS = include/p66_native.h include/overlay_runtime.h tests/p66_native_cases.h
all: build/p66-native-snapshot build/test-p66-native
test: test-p66-native
sanitize: sanitize-p66-native
build/p66-native-snapshot: $(P66_NATIVE_CORE) $(P66_NATIVE_INPUT) platform/host/p66_native_snapshot.c $(P66_NATIVE_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(P66_NATIVE_CORE) $(P66_NATIVE_INPUT) platform/host/p66_native_snapshot.c -o $@
build/test-p66-native: $(P66_NATIVE_CORE) $(P66_NATIVE_INPUT) tests/p66_native_check.c tests/test_p66_native.c tests/p66_native_vectors.h $(P66_NATIVE_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(P66_NATIVE_CORE) $(P66_NATIVE_INPUT) tests/p66_native_check.c tests/test_p66_native.c -o $@
.PHONY: test-p66-native sanitize-p66-native c64-p66-native
test-p66-native: build/test-p66-native
	./build/test-p66-native
sanitize-p66-native: | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(P66_NATIVE_CORE) $(P66_NATIVE_INPUT) tests/p66_native_check.c tests/test_p66_native.c -o build/test-p66-native-sanitize
	./build/test-p66-native-sanitize
C1541 ?= c1541
build/p66-native.prg: $(P66_NATIVE_CORE) $(P66_NATIVE_HEADERS) platform/c64/p66_native_diagnostic.c platform/c64/p66_native_fixture_input.c platform/c64/p66_native_fixture_check.c platform/c64/p66_native_fixture.h tests/p66_native_inputs.h tests/p66_native_vectors.h tests/p66_native_mixed_vectors.h tools/p66_native_build.py tools/p66_overlay_build.py | build
	python3 tools/p66_native_build.py --cc "$(MOS_CC)" --c1541 "$(C1541)"
c64-p66-native: build/p66-native.prg
