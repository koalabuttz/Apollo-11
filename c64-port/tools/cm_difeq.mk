# SPDX-License-Identifier: MIT
CM_DIFEQ_CORE = src/agc_word.c src/agc_multiword.c src/guidance_shift.c src/guidance_vector.c src/guidance_product.c src/guidance_sum.c src/cm_difeq.c
CM_DIFEQ_INPUT = tests/cm_difeq_cases.c
CM_DIFEQ_TEST = $(CM_DIFEQ_INPUT) tests/frozen_words.c tests/test_cm_difeq.c
.PHONY: cm-difeq-host cm-difeq-sanitize cm-difeq-c64
cm-difeq-host:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_DIFEQ_CORE) $(CM_DIFEQ_TEST) -o build/test-cm-difeq
	build/test-cm-difeq
cm-difeq-sanitize:
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_DIFEQ_CORE) $(CM_DIFEQ_INPUT) tools/cm_difeq_snapshot.c -o build/cm-difeq-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/cm-difeq-sanitize > build/cm-difeq-sanitize.snapshots
	cmp build/cm-difeq-reference.snapshots build/cm-difeq-sanitize.snapshots
cm-difeq-c64:
	$(MOS_CC) $(CPPFLAGS) -DCM_DIFEQ_CHECK_ONLY -DCM_DIFEQ_SELECTOR=$(SELECTOR) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(CM_DIFEQ_CORE) $(CM_DIFEQ_TEST) tools/cm_difeq_diagnostic.c -Wl,-Map,build/cm-difeq-selector$(SELECTOR).map -o build/cm-difeq-selector$(SELECTOR).prg

SELECTOR ?= 0
.PHONY: test-cm-difeq
all: build/cm-difeq-native
test: cm-difeq-host
sanitize: cm-difeq-sanitize
test-cm-difeq: cm-difeq-host
build/cm-difeq-native: $(CM_DIFEQ_CORE) $(CM_DIFEQ_INPUT) tools/cm_difeq_snapshot.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_DIFEQ_CORE) $(CM_DIFEQ_INPUT) tools/cm_difeq_snapshot.c -o $@
