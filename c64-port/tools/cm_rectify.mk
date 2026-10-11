# SPDX-License-Identifier: MIT
CM_RECTIFY_CORE = src/agc_word.c src/agc_multiword.c src/guidance_shift.c src/guidance_vector.c src/cm_rectify.c
CM_RECTIFY_INPUT = tests/cm_rectify_cases.c
CM_RECTIFY_TEST = $(CM_RECTIFY_INPUT) tests/frozen_words.c tests/test_cm_rectify.c
.PHONY: cm-rectify-host cm-rectify-sanitize cm-rectify-c64
cm-rectify-host:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_RECTIFY_CORE) $(CM_RECTIFY_TEST) -o build/test-cm-rectify
	build/test-cm-rectify
cm-rectify-sanitize:
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_RECTIFY_CORE) $(CM_RECTIFY_INPUT) tools/cm_rectify_snapshot.c -o build/cm-rectify-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/cm-rectify-sanitize > build/cm-rectify-sanitize.snapshots
	cmp build/cm-rectify-reference.snapshots build/cm-rectify-sanitize.snapshots
cm-rectify-c64:
	$(MOS_CC) $(CPPFLAGS) -DCM_RECTIFY_CHECK_ONLY -DCM_RECTIFY_PHASE_SELECTED=$(if $(PHASE),$(PHASE),0) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(CM_RECTIFY_CORE) $(CM_RECTIFY_TEST) tools/cm_rectify_diagnostic.c -Wl,-Map,build/cm-rectify-phase$(if $(PHASE),$(PHASE),0).map -o build/cm-rectify-phase$(if $(PHASE),$(PHASE),0).prg

.PHONY: test-cm-rectify
test-cm-rectify: cm-rectify-host
test: cm-rectify-host
sanitize: cm-rectify-sanitize
all: build/cm-rectify-snapshot
build/cm-rectify-snapshot: $(CM_RECTIFY_CORE) $(CM_RECTIFY_INPUT) tools/cm_rectify_snapshot.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_RECTIFY_CORE) $(CM_RECTIFY_INPUT) tools/cm_rectify_snapshot.c -o $@
