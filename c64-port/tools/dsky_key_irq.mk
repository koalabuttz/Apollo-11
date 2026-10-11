DSKY_KEY_IRQ_CORE = src/agc_word.c src/executive.c src/dsky_keycom.c src/dsky_keyrupt.c src/dsky_key_irq.c
DSKY_KEY_IRQ_TEST = tests/dsky_key_irq_cases.c tests/dsky_key_irq_check.c
.PHONY: test-dsky-key-irq sanitize-dsky-key-irq c64-dsky-key-irq differential-dsky-key-irq
build/test-dsky-key-irq: $(DSKY_KEY_IRQ_CORE) $(DSKY_KEY_IRQ_TEST) tests/test_dsky_key_irq.c tests/dsky_key_irq_vectors.h
	@mkdir -p build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_KEY_IRQ_CORE) $(DSKY_KEY_IRQ_TEST) tests/test_dsky_key_irq.c -o build/test-dsky-key-irq
test-dsky-key-irq: build/test-dsky-key-irq
	./build/test-dsky-key-irq
sanitize-dsky-key-irq:
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(DSKY_KEY_IRQ_CORE) $(DSKY_KEY_IRQ_TEST) tests/test_dsky_key_irq.c -o build/test-dsky-key-irq-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-key-irq-san
c64-dsky-key-irq:
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(DSKY_KEY_IRQ_CORE) $(DSKY_KEY_IRQ_TEST) tools/dsky_key_irq_diagnostic.c -Wl,-Map,build/dsky_key_irq.map -o build/dsky_key_irq.prg
differential-dsky-key-irq:
	python3 tools/dsky_key_irq_differential.py deps/fixture
	python3 tools/freeze_dsky_key_irq.py
test: test-dsky-key-irq
sanitize: sanitize-dsky-key-irq

.PHONY: all
all: build/test-dsky-key-irq
