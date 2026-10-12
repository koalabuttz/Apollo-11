# SPDX-License-Identifier: MIT
# Milestone lm-periapo (lane E): Translate the Luminary apsides display subroutines (PERIAPO, SHIFTR1, MAXCHK)
LM_PERIAPO_SRC = src/lm_periapo.c src/lm_conic_timerad.c src/lm_conic_timethet.c src/lm_conic_kepler.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/rtb_agreement.c src/fixed_data.c src/agc_word.c src/agc_multiword.c
LM_PERIAPO_TARGET = tests/lm_periapo_target.c tests/lm_periapo_phase_target.c tests/lm_periapo_cases.c tests/lm_periapo_debris.c tests/frozen_words.c
LM_PERIAPO_C64 = tests/lm_periapo_phase_target.c tests/lm_periapo_debris.c tests/frozen_words.c
LM_PERIAPO_HDR = include/lm_periapo.h include/lm_conic_timerad.h include/lm_conic_timethet.h include/lm_conic_kepler.h include/lm_interp_orders.h include/lm_interp_steps.h include/agc_image_ops.h tests/lm_periapo_target.h tests/lm_periapo_vectors.h tests/lm_periapo_phase_target.h tests/lm_periapo_g1_vectors.h tests/lm_periapo_g2_vectors.h tests/lm_periapo_g3_vectors.h tests/lm_periapo_g4_vectors.h tests/lm_periapo_cases.h
.PHONY: test-lm-periapo sanitize-lm-periapo c64-lm-periapo vice-lm-periapo
all: build/test-lm-periapo
test: test-lm-periapo
sanitize: sanitize-lm-periapo
# Oracle stream: unchanged Luminary099 in the pinned yaAGC, checked row by row
# against the native producer and against the pinned stream identity.
build/lm-periapo-reference.snapshots.bin.xz: tools/lm_periapo_differential.py tools/lm_periapo_reference.c $(LM_PERIAPO_SRC) tests/lm_periapo_cases.c tests/lm_periapo_debris.c | build
	python3 tools/lm_periapo_differential.py
build/test-lm-periapo: $(LM_PERIAPO_SRC) $(LM_PERIAPO_TARGET) tests/test_lm_periapo.c $(LM_PERIAPO_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_PERIAPO_SRC) $(LM_PERIAPO_TARGET) tests/test_lm_periapo.c -lm -o $@
test-lm-periapo: build/test-lm-periapo build/lm-periapo-reference.snapshots.bin.xz
	./build/test-lm-periapo
	python3 tools/lm_periapo_host.py --cc "$(CC)"
	python3 tools/lm_periapo_source.py
sanitize-lm-periapo: build/lm-periapo-reference.snapshots.bin.xz | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer $(LM_PERIAPO_SRC) $(LM_PERIAPO_TARGET) tests/test_lm_periapo.c -lm -o build/test-lm-periapo-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-periapo-sanitize
	python3 tools/lm_periapo_host.py --cc "$(CC)" --sanitize
# Four PRGs, one per phase group, with the two images under KERNAL ROM.
c64-lm-periapo: build/lm-periapo-1.prg build/lm-periapo-2.prg build/lm-periapo-3.prg build/lm-periapo-4.prg
build/lm-periapo-%.prg: $(LM_PERIAPO_SRC) $(LM_PERIAPO_C64) platform/c64/lm_periapo_diagnostic.c $(LM_PERIAPO_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DAPOLLO_FIXED_LM_ONLY -DLM_PERIAPO_GROUP=$* -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-periapo-$*.map $(LM_PERIAPO_SRC) $(LM_PERIAPO_C64) platform/c64/lm_periapo_diagnostic.c -o $@
	python3 -c "import re,sys;m=open('build/lm-periapo-$*.map').read();e=max(int(x,16) for x in re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+[0-9a-f]+\s+\d+\s+__heap_start',m,re.M));sys.exit(0 if e<=0xc000 else 'image and BSS reach the mailbox page: '+hex(e))"
vice-lm-periapo: c64-lm-periapo
	python3 tools/vice_run.py build/lm-periapo-1.prg
	python3 tools/vice_run.py build/lm-periapo-2.prg
	python3 tools/vice_run.py build/lm-periapo-3.prg
	python3 tools/vice_run.py build/lm-periapo-4.prg
