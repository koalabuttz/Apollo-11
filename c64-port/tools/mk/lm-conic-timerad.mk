# SPDX-License-Identifier: MIT
# Milestone lm-conic-timerad (lane B): Translate Luminary time-radius and apsides subroutines (TIMERAD, APSIDES)
LM_CONIC_TIMERAD_SRC = src/lm_conic_timerad.c src/lm_conic_timethet.c src/lm_conic_kepler.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/rtb_agreement.c src/fixed_data.c src/agc_word.c src/agc_multiword.c
LM_CONIC_TIMERAD_TARGET = tests/lm_conic_timerad_target.c tests/lm_conic_timerad_phase_target.c tests/lm_conic_timerad_cases.c tests/lm_conic_timerad_debris.c tests/frozen_words.c
LM_CONIC_TIMERAD_C64 = tests/lm_conic_timerad_phase_target.c tests/lm_conic_timerad_debris.c tests/frozen_words.c
LM_CONIC_TIMERAD_HDR = include/lm_conic_timerad.h include/lm_conic_timethet.h include/lm_conic_kepler.h include/lm_interp_orders.h include/lm_interp_steps.h include/agc_image_ops.h tests/lm_conic_timerad_target.h tests/lm_conic_timerad_vectors.h tests/lm_conic_timerad_phase_target.h tests/lm_conic_timerad_g1_vectors.h tests/lm_conic_timerad_g2_vectors.h tests/lm_conic_timerad_g3_vectors.h tests/lm_conic_timerad_g4_vectors.h tests/lm_conic_timerad_g5_vectors.h tests/lm_conic_timerad_g6_vectors.h tests/lm_conic_timerad_cases.h
.PHONY: test-lm-conic-timerad sanitize-lm-conic-timerad c64-lm-conic-timerad vice-lm-conic-timerad
all: build/test-lm-conic-timerad
test: test-lm-conic-timerad
sanitize: sanitize-lm-conic-timerad
# Oracle stream: unchanged Luminary099 in the pinned yaAGC, checked row by row
# against the native producer and against the pinned stream identity.
build/lm-conic-timerad-reference.snapshots.bin.xz: tools/lm_conic_timerad_differential.py tools/lm_conic_timerad_reference.c $(LM_CONIC_TIMERAD_SRC) tests/lm_conic_timerad_cases.c tests/lm_conic_timerad_debris.c | build
	python3 tools/lm_conic_timerad_differential.py
build/test-lm-conic-timerad: $(LM_CONIC_TIMERAD_SRC) $(LM_CONIC_TIMERAD_TARGET) tests/test_lm_conic_timerad.c $(LM_CONIC_TIMERAD_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_CONIC_TIMERAD_SRC) $(LM_CONIC_TIMERAD_TARGET) tests/test_lm_conic_timerad.c -lm -o $@
test-lm-conic-timerad: build/test-lm-conic-timerad build/lm-conic-timerad-reference.snapshots.bin.xz
	./build/test-lm-conic-timerad
	python3 tools/lm_conic_timerad_host.py --cc "$(CC)"
	python3 tools/lm_conic_timerad_source.py
sanitize-lm-conic-timerad: build/lm-conic-timerad-reference.snapshots.bin.xz | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer $(LM_CONIC_TIMERAD_SRC) $(LM_CONIC_TIMERAD_TARGET) tests/test_lm_conic_timerad.c -lm -o build/test-lm-conic-timerad-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-conic-timerad-sanitize
	python3 tools/lm_conic_timerad_host.py --cc "$(CC)" --sanitize
# Six PRGs, one per phase group, with the two images under KERNAL ROM.
c64-lm-conic-timerad: build/lm-conic-timerad-1.prg build/lm-conic-timerad-2.prg build/lm-conic-timerad-3.prg build/lm-conic-timerad-4.prg build/lm-conic-timerad-5.prg build/lm-conic-timerad-6.prg
build/lm-conic-timerad-%.prg: $(LM_CONIC_TIMERAD_SRC) $(LM_CONIC_TIMERAD_C64) platform/c64/lm_conic_timerad_diagnostic.c $(LM_CONIC_TIMERAD_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DAPOLLO_FIXED_LM_ONLY -DLM_CONIC_TIMERAD_GROUP=$* -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-conic-timerad-$*.map $(LM_CONIC_TIMERAD_SRC) $(LM_CONIC_TIMERAD_C64) platform/c64/lm_conic_timerad_diagnostic.c -o $@
	python3 -c "import re,sys;m=open('build/lm-conic-timerad-$*.map').read();e=max(int(x,16) for x in re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+[0-9a-f]+\s+\d+\s+__heap_start',m,re.M));sys.exit(0 if e<=0xc000 else 'image and BSS reach the mailbox page: '+hex(e))"
vice-lm-conic-timerad: c64-lm-conic-timerad
	python3 tools/vice_run.py build/lm-conic-timerad-1.prg
	python3 tools/vice_run.py build/lm-conic-timerad-2.prg
	python3 tools/vice_run.py build/lm-conic-timerad-3.prg
	python3 tools/vice_run.py build/lm-conic-timerad-4.prg
	python3 tools/vice_run.py build/lm-conic-timerad-5.prg
	python3 tools/vice_run.py build/lm-conic-timerad-6.prg
