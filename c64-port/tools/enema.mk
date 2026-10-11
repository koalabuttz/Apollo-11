ENEMA_CORE = src/agc_word.c src/executive.c src/waitlist.c src/restart.c src/enema.c
ENEMA_INPUT = tests/restart_cases.c tests/enema_cases.c
ENEMA_TEST = $(ENEMA_INPUT) tests/enema_check.c
ENEMA_HEADERS = $(RESTART_HEADERS) include/enema.h tests/enema_cases.h tests/enema_check.h tests/enema_vectors.h
all: build/enema-snapshot build/test-enema
build/enema-snapshot: $(ENEMA_CORE) $(ENEMA_INPUT) platform/host/enema_snapshot.c $(ENEMA_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(ENEMA_CORE) $(ENEMA_INPUT) platform/host/enema_snapshot.c -o $@
build/test-enema: $(ENEMA_CORE) $(ENEMA_TEST) tests/test_enema.c $(ENEMA_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(ENEMA_CORE) $(ENEMA_TEST) tests/test_enema.c -o $@
test: test-enema
.PHONY: test-enema sanitize-enema c64-enema
test-enema: build/test-enema
	./build/test-enema
sanitize: sanitize-enema
sanitize-enema: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(ENEMA_CORE) $(ENEMA_TEST) tests/test_enema.c -o build/test-enema-sanitize
	./build/test-enema-sanitize
c64-enema: build/enema.prg
build/enema.prg: $(ENEMA_CORE) $(ENEMA_TEST) platform/c64/enema_diagnostic.c $(ENEMA_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/enema.map $(ENEMA_CORE) $(ENEMA_TEST) platform/c64/enema_diagnostic.c -o $@
