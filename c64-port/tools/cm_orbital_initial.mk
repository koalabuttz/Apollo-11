# SPDX-License-Identifier: MIT
CM_ORBITAL_INITIAL_CORE = src/agc_word.c src/cm_orbital_initial.c
CM_ORBITAL_INITIAL_INPUT = tests/cm_orbital_initial_cases.c
CM_ORBITAL_INITIAL_TEST = $(CM_ORBITAL_INITIAL_INPUT) tests/frozen_words.c tests/test_cm_orbital_initial.c
.PHONY: cm-orbital-initial-host cm-orbital-initial-sanitize cm-orbital-initial-c64
cm-orbital-initial-host:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_ORBITAL_INITIAL_CORE) $(CM_ORBITAL_INITIAL_TEST) -o build/test-cm-orbital-initial
	build/test-cm-orbital-initial
cm-orbital-initial-sanitize:
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_ORBITAL_INITIAL_CORE) $(CM_ORBITAL_INITIAL_INPUT) tools/cm_orbital_initial_snapshot.c -o build/cm-orbital-initial-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/cm-orbital-initial-sanitize > build/cm-orbital-initial-sanitize.snapshots
	cmp build/cm-orbital-initial-reference.snapshots build/cm-orbital-initial-sanitize.snapshots
cm-orbital-initial-c64:
	$(MOS_CC) $(CPPFLAGS) -DCM_ORBITAL_INITIAL_CHECK_ONLY -DCM_ORBITAL_INITIAL_PHASE_SELECTED=$(if $(PHASE),$(PHASE),0) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(CM_ORBITAL_INITIAL_CORE) $(CM_ORBITAL_INITIAL_TEST) tools/cm_orbital_initial_diagnostic.c -Wl,-Map,build/cm-orbital-initial-phase$(if $(PHASE),$(PHASE),0).map -o build/cm-orbital-initial-phase$(if $(PHASE),$(PHASE),0).prg

.PHONY: test-cm-orbital-initial
test-cm-orbital-initial: cm-orbital-initial-host
test: cm-orbital-initial-host
sanitize: cm-orbital-initial-sanitize
all: build/cm-orbital-initial-snapshot
build/cm-orbital-initial-snapshot: $(CM_ORBITAL_INITIAL_CORE) $(CM_ORBITAL_INITIAL_INPUT) tools/cm_orbital_initial_snapshot.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_ORBITAL_INITIAL_CORE) $(CM_ORBITAL_INITIAL_INPUT) tools/cm_orbital_initial_snapshot.c -o $@

PHASE ?= 0
.PHONY: test-cm-orbital-initial
all: build/cm-orbital-initial-native
test: cm-orbital-initial-host
sanitize: cm-orbital-initial-sanitize
test-cm-orbital-initial: cm-orbital-initial-host
build/cm-orbital-initial-native: $(CM_ORBITAL_INITIAL_CORE) $(CM_ORBITAL_INITIAL_INPUT) tools/cm_orbital_initial_snapshot.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_ORBITAL_INITIAL_CORE) $(CM_ORBITAL_INITIAL_INPUT) tools/cm_orbital_initial_snapshot.c -o $@
