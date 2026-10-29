RECOVERY_CORE = src/agc_word.c src/recovery.c
RECOVERY_INPUT = tests/recovery_cases.c
RECOVERY_TEST = $(RECOVERY_INPUT) tests/recovery_check.c
RECOVERY_HEADERS = $(RESTART_HEADERS) include/recovery.h tests/recovery_cases.h tests/recovery_check.h tests/recovery_vectors.h
all: build/recovery-snapshot build/test-recovery
build/recovery-snapshot: $(RECOVERY_CORE) $(RECOVERY_INPUT) platform/host/recovery_snapshot.c $(RECOVERY_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(RECOVERY_CORE) $(RECOVERY_INPUT) platform/host/recovery_snapshot.c -o $@
build/test-recovery: $(RECOVERY_CORE) $(RECOVERY_TEST) tests/test_recovery.c $(RECOVERY_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(RECOVERY_CORE) $(RECOVERY_TEST) tests/test_recovery.c -o $@
test: test-recovery
.PHONY: test-recovery sanitize-recovery c64-recovery
test-recovery: build/test-recovery
	./build/test-recovery
sanitize: sanitize-recovery
sanitize-recovery: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(RECOVERY_CORE) $(RECOVERY_TEST) tests/test_recovery.c -o build/test-recovery-sanitize
	./build/test-recovery-sanitize
c64-recovery: build/recovery.prg
build/recovery.prg: $(RECOVERY_CORE) $(RECOVERY_TEST) platform/c64/recovery_diagnostic.c $(RECOVERY_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/recovery.map $(RECOVERY_CORE) $(RECOVERY_TEST) platform/c64/recovery_diagnostic.c -o $@
