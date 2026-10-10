CC = cc
MOS_CC ?= mos-c64-clang
CFLAGS ?= -O2 -g -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic
CPPFLAGS = -Iinclude -Itests
CORE = src/agc_word.c src/executive.c src/native_dispatch.c src/irq_delivery.c
SCENARIOS = tests/scenarios.c tests/state_digest.c tests/dispatch_cases.c tests/dispatch_check.c
HEADERS = include/agc_word.h include/executive.h tests/scenarios.h tests/state_digest.h tests/reference_vectors.h include/native_dispatch.h include/irq_delivery.h tests/dispatch_cases.h tests/dispatch_check.h
.PHONY: all test c64 sanitize clean differential
all: build/test-irq build/test-host build/test-lifecycle build/snapshot build/arithmetic-dump build/dispatch-snapshot build/lifecycle-snapshot
build:
	mkdir -p build
build/test-host: $(CORE) $(SCENARIOS) tests/test_host.c $(HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CORE) $(SCENARIOS) tests/test_host.c -o $@
build/snapshot: $(CORE) $(SCENARIOS) platform/host/snapshot.c $(HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CORE) $(SCENARIOS) platform/host/snapshot.c -o $@
build/arithmetic-dump: src/agc_word.c tests/arithmetic_dump.c tests/arithmetic_stream.h include/agc_word.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) src/agc_word.c tests/arithmetic_dump.c -o $@
test: all
	./build/test-host
	./build/test-irq
	./build/test-lifecycle
sanitize: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CORE) $(SCENARIOS) tests/test_host.c -o build/test-sanitize
	./build/test-sanitize
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CORE) tests/scenarios.c tests/test_irq.c -o build/test-irq-sanitize
	./build/test-irq-sanitize
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CORE) $(SCENARIOS) $(LIFECYCLE) tests/lifecycle_check.c tests/test_lifecycle.c -o build/test-lifecycle-sanitize
	./build/test-lifecycle-sanitize
c64: build/executive.prg
build/executive.prg: $(CORE) $(SCENARIOS) platform/c64/main.c $(HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -Wall -Wextra -Werror -Wl,-Map,build/executive.map $(CORE) $(SCENARIOS) platform/c64/main.c -o $@
differential: all
	python3 tools/differential.py --reference "$(REFERENCE)"
clean:
	rm -rf build

build/dispatch-snapshot: $(CORE) $(SCENARIOS) platform/host/dispatch_snapshot.c $(HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CORE) $(SCENARIOS) platform/host/dispatch_snapshot.c -o $@

build/test-irq: $(CORE) tests/scenarios.c tests/test_irq.c $(HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CORE) tests/scenarios.c tests/test_irq.c -o $@

LIFECYCLE = tests/lifecycle_cases.c
build/lifecycle-snapshot: $(CORE) $(SCENARIOS) $(LIFECYCLE) platform/host/lifecycle_snapshot.c $(HEADERS) tests/lifecycle_cases.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CORE) $(SCENARIOS) $(LIFECYCLE) platform/host/lifecycle_snapshot.c -o $@
build/test-lifecycle: $(CORE) $(SCENARIOS) $(LIFECYCLE) tests/lifecycle_check.c tests/test_lifecycle.c $(HEADERS) tests/lifecycle_cases.h tests/lifecycle_check.h tests/lifecycle_vectors.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CORE) $(SCENARIOS) $(LIFECYCLE) tests/lifecycle_check.c tests/test_lifecycle.c -o $@

.PHONY: c64-irq
c64-irq: build/irq.prg
build/irq.prg: $(CORE) $(SCENARIOS) platform/c64/irq_diagnostic.c platform/c64/irq_platform.c platform/c64/irq_platform.h platform/c64/irq_gate.S $(HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -Iplatform/c64 -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/irq.map $(CORE) $(SCENARIOS) platform/c64/irq_diagnostic.c platform/c64/irq_platform.c platform/c64/irq_gate.S -o $@

.PHONY: c64-lifecycle
c64-lifecycle: build/lifecycle.prg
build/lifecycle.prg: $(CORE) $(SCENARIOS) $(LIFECYCLE) tests/lifecycle_check.c platform/c64/lifecycle_diagnostic.c platform/c64/irq_platform.c platform/c64/irq_platform.h platform/c64/irq_gate.S $(HEADERS) tests/lifecycle_cases.h tests/lifecycle_check.h tests/lifecycle_vectors.h | build
	$(MOS_CC) $(CPPFLAGS) -Iplatform/c64 -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lifecycle.map $(CORE) $(SCENARIOS) $(LIFECYCLE) tests/lifecycle_check.c platform/c64/lifecycle_diagnostic.c platform/c64/irq_platform.c platform/c64/irq_gate.S -o $@
