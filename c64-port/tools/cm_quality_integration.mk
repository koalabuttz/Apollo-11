# SPDX-License-Identifier: MIT
CM_QUALITY_INTEGRATED_CORE = tests/cm_quality_cases.c src/cm_quality.c src/interpreted_square.c src/guidance_product.c src/guidance_sum.c src/guidance_shift.c src/agc_word.c src/agc_multiword.c
all: build/cm-quality-integrated-stream
build/cm-quality-integrated-stream: tools/cm_quality_snapshot.c $(CM_QUALITY_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test: test-cm-quality-integrated
test-cm-quality-integrated:
	python3 tools/cm_quality_host.py
	for phase in $$(seq 0 19); do $(CC) $(CPPFLAGS) $(CFLAGS) -DCMQ_PHASE=$$phase tests/test_cm_quality.c tests/frozen_words.c $(CM_QUALITY_INTEGRATED_CORE) -o build/test-cm-quality-integrated-$$phase; build/test-cm-quality-integrated-$$phase || exit $$?; done
sanitize: sanitize-cm-quality-integrated
sanitize-cm-quality-integrated:
	python3 tools/cm_quality_host.py --sanitize
	for phase in $$(seq 0 19); do $(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie -DCMQ_PHASE=$$phase tests/test_cm_quality.c tests/frozen_words.c $(CM_QUALITY_INTEGRATED_CORE) -o build/test-cm-quality-integrated-$$phase-san; ASAN_OPTIONS=detect_leaks=0 build/test-cm-quality-integrated-$$phase-san || exit $$?; done
.PHONY: test-cm-quality-integrated sanitize-cm-quality-integrated
