# SPDX-License-Identifier: MIT
# Milestone lm-conic-kepler (lane B): Translate Luminary Kepler subroutine (KEPLERN, DELTIME, CHECKCTR)
LM_CONIC_KEPLER_SRC = src/lm_conic_kepler.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/rtb_agreement.c src/fixed_data.c src/agc_word.c src/agc_multiword.c
LM_CONIC_KEPLER_TARGET = tests/lm_conic_kepler_target.c tests/lm_conic_kepler_phase_target.c tests/lm_conic_kepler_cases.c tests/lm_conic_kepler_debris.c tests/frozen_words.c
LM_CONIC_KEPLER_C64 = tests/lm_conic_kepler_phase_target.c tests/lm_conic_kepler_debris.c tests/frozen_words.c
LM_CONIC_KEPLER_HDR = include/lm_conic_kepler.h include/lm_interp_orders.h include/lm_interp_steps.h include/agc_image_ops.h tests/lm_conic_kepler_target.h tests/lm_conic_kepler_vectors.h tests/lm_conic_kepler_phase_target.h tests/lm_conic_kepler_g1_vectors.h tests/lm_conic_kepler_g2_vectors.h tests/lm_conic_kepler_g3_vectors.h tests/lm_conic_kepler_g4_vectors.h tests/lm_conic_kepler_cases.h
.PHONY: test-lm-conic-kepler sanitize-lm-conic-kepler c64-lm-conic-kepler vice-lm-conic-kepler
all: build/test-lm-conic-kepler
test: test-lm-conic-kepler
sanitize: sanitize-lm-conic-kepler
# Oracle stream: unchanged Luminary099 in the pinned yaAGC, checked row by row
# against the native producer and against the pinned stream identity.
build/lm-conic-kepler-reference.snapshots.bin.xz: tools/lm_conic_kepler_differential.py tools/lm_conic_kepler_reference.c $(LM_CONIC_KEPLER_SRC) tests/lm_conic_kepler_cases.c tests/lm_conic_kepler_debris.c | build
	python3 tools/lm_conic_kepler_differential.py
build/test-lm-conic-kepler: $(LM_CONIC_KEPLER_SRC) $(LM_CONIC_KEPLER_TARGET) tests/test_lm_conic_kepler.c $(LM_CONIC_KEPLER_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_CONIC_KEPLER_SRC) $(LM_CONIC_KEPLER_TARGET) tests/test_lm_conic_kepler.c -lm -o $@
test-lm-conic-kepler: build/test-lm-conic-kepler build/lm-conic-kepler-reference.snapshots.bin.xz
	./build/test-lm-conic-kepler
	python3 tools/lm_conic_kepler_host.py --cc "$(CC)"
	python3 tools/lm_conic_kepler_source.py
sanitize-lm-conic-kepler: build/lm-conic-kepler-reference.snapshots.bin.xz | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer $(LM_CONIC_KEPLER_SRC) $(LM_CONIC_KEPLER_TARGET) tests/test_lm_conic_kepler.c -lm -o build/test-lm-conic-kepler-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-conic-kepler-sanitize
	python3 tools/lm_conic_kepler_host.py --cc "$(CC)" --sanitize
# Four PRGs, one per phase group, with the two images under KERNAL ROM.
c64-lm-conic-kepler: build/lm-conic-kepler-1.prg build/lm-conic-kepler-2.prg build/lm-conic-kepler-3.prg build/lm-conic-kepler-4.prg
build/lm-conic-kepler-%.prg: $(LM_CONIC_KEPLER_SRC) $(LM_CONIC_KEPLER_C64) platform/c64/lm_conic_kepler_diagnostic.c $(LM_CONIC_KEPLER_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DAPOLLO_FIXED_LM_ONLY -DLM_CONIC_KEPLER_GROUP=$* -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-conic-kepler-$*.map $(LM_CONIC_KEPLER_SRC) $(LM_CONIC_KEPLER_C64) platform/c64/lm_conic_kepler_diagnostic.c -o $@
	python3 -c "import re,sys;m=open('build/lm-conic-kepler-$*.map').read();e=max(int(x,16) for x in re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+[0-9a-f]+\s+\d+\s+__heap_start',m,re.M));sys.exit(0 if e<=0xc000 else 'image and BSS reach the mailbox page: '+hex(e))"
vice-lm-conic-kepler: c64-lm-conic-kepler
	python3 tools/vice_run.py build/lm-conic-kepler-1.prg
	python3 tools/vice_run.py build/lm-conic-kepler-2.prg
	python3 tools/vice_run.py build/lm-conic-kepler-3.prg
	python3 tools/vice_run.py build/lm-conic-kepler-4.prg
