# SPDX-License-Identifier: MIT
# Milestone lm-ephemeris (lane B): Translate Luminary lunar and solar ephemerides (LSPOS/LUNPOS, LUNVEL)
LM_EPHEMERIS_SRC = src/lm_ephemeris.c src/lm_interp_steps.c src/ephemeris_time_scale.c src/ephemeris_add.c src/ephemeris_day.c src/ephemeris_shift.c src/sband_scalar.c src/sband_trig.c src/matrix_native.c src/rtb_unit.c src/fixed_data.c src/agc_word.c src/agc_multiword.c src/guidance_dp_divide.c src/guidance_trig.c src/guidance_product.c src/guidance_length_phased.c src/guidance_unit_phased.c src/rtb_vector.c src/guidance_norm.c src/guidance_shift.c
LM_EPHEMERIS_TARGET = tests/lm_ephemeris_target.c tests/lm_ephemeris_phase_target.c tests/lm_ephemeris_cases.c tests/frozen_words.c
LM_EPHEMERIS_C64 = tests/lm_ephemeris_phase_target.c tests/lm_ephemeris_cases.c tests/frozen_words.c
LM_EPHEMERIS_HDR = include/lm_ephemeris.h include/lm_interp_steps.h tests/lm_ephemeris_target.h tests/lm_ephemeris_vectors.h tests/lm_ephemeris_phase_target.h tests/lm_ephemeris_g1_vectors.h tests/lm_ephemeris_g2_vectors.h tests/lm_ephemeris_g3_vectors.h tests/lm_ephemeris_cases.h
.PHONY: test-lm-ephemeris sanitize-lm-ephemeris c64-lm-ephemeris vice-lm-ephemeris
all: build/test-lm-ephemeris
test: test-lm-ephemeris
sanitize: sanitize-lm-ephemeris
# Oracle stream: unchanged Luminary099 in the pinned yaAGC, checked row by row
# against the native producer and against the pinned stream identity.
build/lm-ephemeris-reference.snapshots.bin.xz: tools/lm_ephemeris_differential.py tools/lm_ephemeris_reference.c $(LM_EPHEMERIS_SRC) | build
	python3 tools/lm_ephemeris_differential.py
build/test-lm-ephemeris: $(LM_EPHEMERIS_SRC) $(LM_EPHEMERIS_TARGET) tests/test_lm_ephemeris.c $(LM_EPHEMERIS_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_EPHEMERIS_SRC) $(LM_EPHEMERIS_TARGET) tests/test_lm_ephemeris.c -o $@
test-lm-ephemeris: build/test-lm-ephemeris build/lm-ephemeris-reference.snapshots.bin.xz
	./build/test-lm-ephemeris
	python3 tools/lm_ephemeris_host.py --cc "$(CC)"
sanitize-lm-ephemeris: build/lm-ephemeris-reference.snapshots.bin.xz | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer $(LM_EPHEMERIS_SRC) $(LM_EPHEMERIS_TARGET) tests/test_lm_ephemeris.c -o build/test-lm-ephemeris-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-ephemeris-sanitize
	python3 tools/lm_ephemeris_host.py --cc "$(CC)" --sanitize
# Three PRGs, one per phase group, with the two images under KERNAL ROM.
c64-lm-ephemeris: build/lm-ephemeris-1.prg build/lm-ephemeris-2.prg build/lm-ephemeris-3.prg
build/lm-ephemeris-%.prg: $(LM_EPHEMERIS_SRC) $(LM_EPHEMERIS_C64) platform/c64/lm_ephemeris_diagnostic.c $(LM_EPHEMERIS_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DAPOLLO_FIXED_LM_ONLY -DLM_EPHEMERIS_GROUP=$* -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-ephemeris-$*.map $(LM_EPHEMERIS_SRC) $(LM_EPHEMERIS_C64) platform/c64/lm_ephemeris_diagnostic.c -o $@
	python3 -c "import re,sys;m=open('build/lm-ephemeris-$*.map').read();e=max(int(x,16) for x in re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+[0-9a-f]+\s+\d+\s+__heap_start',m,re.M));sys.exit(0 if e<=0xc000 else 'image and BSS reach the mailbox page: '+hex(e))"
vice-lm-ephemeris: c64-lm-ephemeris
	python3 tools/vice_run.py build/lm-ephemeris-1.prg
	python3 tools/vice_run.py build/lm-ephemeris-2.prg
	python3 tools/vice_run.py build/lm-ephemeris-3.prg
