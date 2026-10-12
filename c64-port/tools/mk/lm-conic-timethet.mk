# SPDX-License-Identifier: MIT
# Milestone lm-conic-timethet (lane B): Translate Luminary time-theta subroutine (TIMETHET, PARAM, GEOM, GETX, NEWSTATE)
LM_CONIC_TIMETHET_SRC = src/lm_conic_timethet.c src/lm_conic_kepler.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/rtb_agreement.c src/fixed_data.c src/agc_word.c src/agc_multiword.c
LM_CONIC_TIMETHET_TARGET = tests/lm_conic_timethet_target.c tests/lm_conic_timethet_phase_target.c tests/lm_conic_timethet_cases.c tests/lm_conic_timethet_debris.c tests/frozen_words.c
LM_CONIC_TIMETHET_C64 = tests/lm_conic_timethet_phase_target.c tests/lm_conic_timethet_debris.c tests/frozen_words.c
LM_CONIC_TIMETHET_HDR = include/lm_conic_timethet.h include/lm_conic_kepler.h include/lm_interp_orders.h include/lm_interp_steps.h include/agc_image_ops.h tests/lm_conic_timethet_target.h tests/lm_conic_timethet_vectors.h tests/lm_conic_timethet_phase_target.h tests/lm_conic_timethet_g1_vectors.h tests/lm_conic_timethet_g2_vectors.h tests/lm_conic_timethet_g3_vectors.h tests/lm_conic_timethet_g4_vectors.h tests/lm_conic_timethet_cases.h
.PHONY: test-lm-conic-timethet sanitize-lm-conic-timethet c64-lm-conic-timethet vice-lm-conic-timethet
all: build/test-lm-conic-timethet
test: test-lm-conic-timethet
sanitize: sanitize-lm-conic-timethet
# Oracle stream: unchanged Luminary099 in the pinned yaAGC, checked row by row
# against the native producer and against the pinned stream identity.
build/lm-conic-timethet-reference.snapshots.bin.xz: tools/lm_conic_timethet_differential.py tools/lm_conic_timethet_reference.c $(LM_CONIC_TIMETHET_SRC) tests/lm_conic_timethet_cases.c tests/lm_conic_timethet_debris.c | build
	python3 tools/lm_conic_timethet_differential.py
build/test-lm-conic-timethet: $(LM_CONIC_TIMETHET_SRC) $(LM_CONIC_TIMETHET_TARGET) tests/test_lm_conic_timethet.c $(LM_CONIC_TIMETHET_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_CONIC_TIMETHET_SRC) $(LM_CONIC_TIMETHET_TARGET) tests/test_lm_conic_timethet.c -lm -o $@
test-lm-conic-timethet: build/test-lm-conic-timethet build/lm-conic-timethet-reference.snapshots.bin.xz
	./build/test-lm-conic-timethet
	python3 tools/lm_conic_timethet_host.py --cc "$(CC)"
	python3 tools/lm_conic_timethet_source.py
sanitize-lm-conic-timethet: build/lm-conic-timethet-reference.snapshots.bin.xz | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer $(LM_CONIC_TIMETHET_SRC) $(LM_CONIC_TIMETHET_TARGET) tests/test_lm_conic_timethet.c -lm -o build/test-lm-conic-timethet-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-conic-timethet-sanitize
	python3 tools/lm_conic_timethet_host.py --cc "$(CC)" --sanitize
# Four PRGs, one per phase group, with the two images under KERNAL ROM.
c64-lm-conic-timethet: build/lm-conic-timethet-1.prg build/lm-conic-timethet-2.prg build/lm-conic-timethet-3.prg build/lm-conic-timethet-4.prg
build/lm-conic-timethet-%.prg: $(LM_CONIC_TIMETHET_SRC) $(LM_CONIC_TIMETHET_C64) platform/c64/lm_conic_timethet_diagnostic.c $(LM_CONIC_TIMETHET_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DAPOLLO_FIXED_LM_ONLY -DLM_CONIC_TIMETHET_GROUP=$* -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-conic-timethet-$*.map $(LM_CONIC_TIMETHET_SRC) $(LM_CONIC_TIMETHET_C64) platform/c64/lm_conic_timethet_diagnostic.c -o $@
	python3 -c "import re,sys;m=open('build/lm-conic-timethet-$*.map').read();e=max(int(x,16) for x in re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+[0-9a-f]+\s+\d+\s+__heap_start',m,re.M));sys.exit(0 if e<=0xc000 else 'image and BSS reach the mailbox page: '+hex(e))"
vice-lm-conic-timethet: c64-lm-conic-timethet
	python3 tools/vice_run.py build/lm-conic-timethet-1.prg
	python3 tools/vice_run.py build/lm-conic-timethet-2.prg
	python3 tools/vice_run.py build/lm-conic-timethet-3.prg
	python3 tools/vice_run.py build/lm-conic-timethet-4.prg
