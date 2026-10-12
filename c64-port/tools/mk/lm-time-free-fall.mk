# SPDX-License-Identifier: MIT
# Milestone lm-time-free-fall (lane B): Translate Luminary time of free fall (TFFCONIC/TFFCONMU, TFFRP/RA, CALCTPER, CALCTFF)
LM_TIME_FREE_FALL_SRC = src/lm_time_free_fall.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/rtb_agreement.c src/fixed_data.c src/agc_word.c src/agc_multiword.c
LM_TIME_FREE_FALL_TARGET = tests/lm_time_free_fall_target.c tests/lm_time_free_fall_phase_target.c tests/lm_time_free_fall_cases.c tests/lm_time_free_fall_debris.c tests/frozen_words.c
LM_TIME_FREE_FALL_C64 = tests/lm_time_free_fall_phase_target.c tests/lm_time_free_fall_debris.c tests/frozen_words.c
LM_TIME_FREE_FALL_HDR = include/lm_time_free_fall.h include/lm_interp_orders.h include/lm_interp_steps.h include/agc_image_ops.h tests/lm_time_free_fall_target.h tests/lm_time_free_fall_vectors.h tests/lm_time_free_fall_phase_target.h tests/lm_time_free_fall_g1_vectors.h tests/lm_time_free_fall_g2_vectors.h tests/lm_time_free_fall_g3_vectors.h tests/lm_time_free_fall_g4_vectors.h tests/lm_time_free_fall_cases.h
.PHONY: test-lm-time-free-fall sanitize-lm-time-free-fall c64-lm-time-free-fall vice-lm-time-free-fall
all: build/test-lm-time-free-fall
test: test-lm-time-free-fall
sanitize: sanitize-lm-time-free-fall
# Oracle stream: unchanged Luminary099 in the pinned yaAGC, checked row by row
# against the native producer and against the pinned stream identity.
build/lm-time-free-fall-reference.snapshots.bin.xz: tools/lm_time_free_fall_differential.py tools/lm_time_free_fall_reference.c $(LM_TIME_FREE_FALL_SRC) tests/lm_time_free_fall_cases.c tests/lm_time_free_fall_debris.c | build
	python3 tools/lm_time_free_fall_differential.py
build/test-lm-time-free-fall: $(LM_TIME_FREE_FALL_SRC) $(LM_TIME_FREE_FALL_TARGET) tests/test_lm_time_free_fall.c $(LM_TIME_FREE_FALL_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_TIME_FREE_FALL_SRC) $(LM_TIME_FREE_FALL_TARGET) tests/test_lm_time_free_fall.c -o $@
test-lm-time-free-fall: build/test-lm-time-free-fall build/lm-time-free-fall-reference.snapshots.bin.xz
	./build/test-lm-time-free-fall
	python3 tools/lm_time_free_fall_host.py --cc "$(CC)"
	python3 tools/lm_time_free_fall_source.py
	python3 tools/lmi_body_differential.py --cases 400
sanitize-lm-time-free-fall: build/lm-time-free-fall-reference.snapshots.bin.xz | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer $(LM_TIME_FREE_FALL_SRC) $(LM_TIME_FREE_FALL_TARGET) tests/test_lm_time_free_fall.c -o build/test-lm-time-free-fall-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-time-free-fall-sanitize
	python3 tools/lm_time_free_fall_host.py --cc "$(CC)" --sanitize
# Four PRGs, one per phase group, with the two images under KERNAL ROM.
c64-lm-time-free-fall: build/lm-time-free-fall-1.prg build/lm-time-free-fall-2.prg build/lm-time-free-fall-3.prg build/lm-time-free-fall-4.prg
build/lm-time-free-fall-%.prg: $(LM_TIME_FREE_FALL_SRC) $(LM_TIME_FREE_FALL_C64) platform/c64/lm_time_free_fall_diagnostic.c $(LM_TIME_FREE_FALL_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DAPOLLO_FIXED_LM_ONLY -DLM_TIME_FREE_FALL_GROUP=$* -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-time-free-fall-$*.map $(LM_TIME_FREE_FALL_SRC) $(LM_TIME_FREE_FALL_C64) platform/c64/lm_time_free_fall_diagnostic.c -o $@
	python3 -c "import re,sys;m=open('build/lm-time-free-fall-$*.map').read();e=max(int(x,16) for x in re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+[0-9a-f]+\s+\d+\s+__heap_start',m,re.M));sys.exit(0 if e<=0xc000 else 'image and BSS reach the mailbox page: '+hex(e))"
vice-lm-time-free-fall: c64-lm-time-free-fall
	python3 tools/vice_run.py build/lm-time-free-fall-1.prg
	python3 tools/vice_run.py build/lm-time-free-fall-2.prg
	python3 tools/vice_run.py build/lm-time-free-fall-3.prg
	python3 tools/vice_run.py build/lm-time-free-fall-4.prg
