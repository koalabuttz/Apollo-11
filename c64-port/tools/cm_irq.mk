CM_IRQ_CORE = src/agc_word.c src/cm_executive.c src/cm_dispatch.c src/cm_native_dispatch.c src/cm_irq.c
CM_IRQ_INPUT = tests/cm_executive_cases.c tests/cm_dispatch_cases.c tests/cm_irq_cases.c tests/cm_irq_sequence.c
CM_IRQ_HEADERS = include/agc_word.h include/executive.h include/native_dispatch.h include/irq_delivery.h include/cm_executive.h include/cm_dispatch.h include/cm_native_dispatch.h tests/cm_executive_cases.h tests/cm_dispatch_cases.h include/cm_irq.h tests/cm_irq_cases.h tests/cm_irq_sequence.h tests/cm_irq_check.h tests/cm_irq_vectors.h
all: build/cm-irq-snapshot build/cm-irq-sequence-snapshot build/test-cm-irq
build/cm-irq-snapshot: $(CM_IRQ_CORE) $(CM_IRQ_INPUT) platform/host/cm_irq_snapshot.c $(CM_IRQ_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_IRQ_CORE) $(CM_IRQ_INPUT) platform/host/cm_irq_snapshot.c -o $@
build/cm-irq-sequence-snapshot: $(CM_IRQ_CORE) $(CM_IRQ_INPUT) platform/host/cm_irq_sequence_snapshot.c $(CM_IRQ_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_IRQ_CORE) $(CM_IRQ_INPUT) platform/host/cm_irq_sequence_snapshot.c -o $@
build/test-cm-irq: $(CM_IRQ_CORE) $(CM_IRQ_INPUT) tests/cm_irq_check.c tests/test_cm_irq.c $(CM_IRQ_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_IRQ_CORE) $(CM_IRQ_INPUT) tests/cm_irq_check.c tests/test_cm_irq.c -o $@
.PHONY: cm-irq-differential test-cm-irq sanitize-cm-irq c64-cm-irq
cm-irq-differential: build/cm-irq-snapshot build/cm-irq-sequence-snapshot
	python3 tools/cm_irq_differential.py
test: test-cm-irq
test-cm-irq: build/test-cm-irq
	./build/test-cm-irq
sanitize: sanitize-cm-irq
sanitize-cm-irq: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_IRQ_CORE) $(CM_IRQ_INPUT) tests/cm_irq_check.c tests/test_cm_irq.c -o build/test-cm-irq-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-irq-sanitize
c64-cm-irq: build/cm-irq.prg build/cm-irq-admission.prg
build/cm-irq.prg: $(CM_IRQ_CORE) $(CM_IRQ_INPUT) tests/cm_irq_check.c platform/c64/cm_irq_diagnostic.c platform/c64/cm_irq_platform.c platform/c64/cm_irq_gate.S $(CM_IRQ_HEADERS) platform/c64/cm_irq_platform.h | build
	$(MOS_CC) $(CPPFLAGS) -Iplatform/c64 -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-irq.map $(CM_IRQ_CORE) $(CM_IRQ_INPUT) tests/cm_irq_check.c platform/c64/cm_irq_diagnostic.c platform/c64/cm_irq_platform.c platform/c64/cm_irq_gate.S -o $@

build/cm-irq-admission.prg: $(CM_IRQ_CORE) $(CM_IRQ_INPUT) tests/cm_irq_check.c platform/c64/cm_irq_admission_diagnostic.c $(CM_IRQ_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-irq-admission.map $(CM_IRQ_CORE) $(CM_IRQ_INPUT) tests/cm_irq_check.c platform/c64/cm_irq_admission_diagnostic.c -o $@
