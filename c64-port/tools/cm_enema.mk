CM_ENEMA_CORE = src/agc_word.c src/cm_executive.c src/cm_waitlist.c src/cm_restart.c src/cm_enema.c
CM_ENEMA_INPUT = tests/cm_restart_cases.c tests/cm_enema_cases.c
CM_ENEMA_HEADERS = include/cm_enema.h include/cm_restart.h include/cm_executive.h include/cm_waitlist.h tests/cm_restart_cases.h tests/cm_enema_cases.h tests/cm_enema_check.h tests/cm_enema_vectors.h
all: build/cm-enema-snapshot build/test-cm-enema
build/cm-enema-snapshot: $(CM_ENEMA_CORE) $(CM_ENEMA_INPUT) platform/host/cm_enema_snapshot.c $(CM_ENEMA_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_ENEMA_CORE) $(CM_ENEMA_INPUT) platform/host/cm_enema_snapshot.c -o $@
build/test-cm-enema: $(CM_ENEMA_CORE) $(CM_ENEMA_INPUT) tests/cm_enema_check.c tests/test_cm_enema.c $(CM_ENEMA_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_ENEMA_CORE) $(CM_ENEMA_INPUT) tests/cm_enema_check.c tests/test_cm_enema.c -o $@
.PHONY: cm-enema-differential test-cm-enema sanitize-cm-enema c64-cm-enema
cm-enema-differential: build/cm-enema-snapshot
	python3 tools/cm_enema_differential.py
test: test-cm-enema
test-cm-enema: build/test-cm-enema
	./build/test-cm-enema
sanitize: sanitize-cm-enema
sanitize-cm-enema: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_ENEMA_CORE) $(CM_ENEMA_INPUT) tests/cm_enema_check.c tests/test_cm_enema.c -o build/test-cm-enema-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-enema-sanitize
c64-cm-enema: build/cm-enema.prg
build/cm-enema.prg: $(CM_ENEMA_CORE) $(CM_ENEMA_INPUT) tests/cm_enema_check.c platform/c64/cm_enema_diagnostic.c $(CM_ENEMA_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-enema.map $(CM_ENEMA_CORE) $(CM_ENEMA_INPUT) tests/cm_enema_check.c platform/c64/cm_enema_diagnostic.c -o $@
