# SPDX-License-Identifier: MIT
# Milestone lm-conic-lambert (lane B): Translate the Luminary Lambert subroutine (LAMBERT, ITERATOR)
LM_CONIC_LAMBERT_SRC = src/lm_conic_lambert.c src/lm_conic_timethet.c src/lm_conic_kepler.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/rtb_agreement.c src/fixed_data.c src/agc_word.c src/agc_multiword.c
LM_CONIC_LAMBERT_TARGET = tests/lm_conic_lambert_target.c tests/lm_conic_lambert_phase_target.c tests/lm_conic_lambert_cases.c tests/lm_conic_lambert_debris.c tests/frozen_words.c
LM_CONIC_LAMBERT_C64 = tests/lm_conic_lambert_phase_target.c tests/lm_conic_lambert_debris.c tests/frozen_words.c
LM_CONIC_LAMBERT_HDR = include/lm_conic_lambert.h include/lm_conic_timethet.h include/lm_conic_kepler.h include/lm_interp_orders.h include/lm_interp_steps.h include/agc_image_ops.h tests/lm_conic_lambert_target.h tests/lm_conic_lambert_vectors.h tests/lm_conic_lambert_phase_target.h tests/lm_conic_lambert_g1_vectors.h tests/lm_conic_lambert_g2_vectors.h tests/lm_conic_lambert_g3_vectors.h tests/lm_conic_lambert_g4_vectors.h tests/lm_conic_lambert_g5_vectors.h tests/lm_conic_lambert_g6_vectors.h tests/lm_conic_lambert_g7_vectors.h tests/lm_conic_lambert_g8_vectors.h tests/lm_conic_lambert_g9_vectors.h tests/lm_conic_lambert_cases.h
.PHONY: test-lm-conic-lambert sanitize-lm-conic-lambert c64-lm-conic-lambert vice-lm-conic-lambert
all: build/test-lm-conic-lambert
# Oracle stream: unchanged Luminary099 in the pinned yaAGC, checked row by row
# against the native producer and against the pinned stream identity.
build/lm-conic-lambert-reference.snapshots.bin.xz: tools/lm_conic_lambert_differential.py tools/lm_conic_lambert_reference.c $(LM_CONIC_LAMBERT_SRC) tests/lm_conic_lambert_cases.c tests/lm_conic_lambert_debris.c | build
	python3 tools/lm_conic_lambert_differential.py
build/test-lm-conic-lambert: $(LM_CONIC_LAMBERT_SRC) $(LM_CONIC_LAMBERT_TARGET) tests/test_lm_conic_lambert.c $(LM_CONIC_LAMBERT_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_CONIC_LAMBERT_SRC) $(LM_CONIC_LAMBERT_TARGET) tests/test_lm_conic_lambert.c -lm -o $@
# The 65536 oracle entries in four parts of consecutive entries, each run in
# the pinned yaAGC and in a fresh strict (or ASAN/UBSAN) native producer,
# compared row by row and against the pinned SHA-256 of the part
# (tools/oracle_stream_part.py, evidence/lm-conic-lambert/lm-conic-lambert-source-stream-parts.json,
# recorded from the whole pinned stream). Part 1 also runs the frozen
# entries and phase steps (and, unsanitized, the source map).
LM_CONIC_LAMBERT_PART_DEPS = tools/oracle_stream_part.py tools/lm_conic_lambert_core.py tools/lm_conic_lambert_reference.c tools/lm_conic_lambert_snapshot.c \
  tools/prepare_hardware_dv.py $(LM_CONIC_LAMBERT_SRC) tests/lm_conic_lambert_cases.c tests/lm_conic_lambert_debris.c tests/cm_executive_cases.c \
  evidence/lm-conic-lambert/lm-conic-lambert-source-stream.json evidence/lm-conic-lambert/lm-conic-lambert-source-stream-parts.json tools/mk/lm-conic-lambert.mk
LM_CONIC_LAMBERT_SANITIZE_BUILD = $(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic \
  -fsanitize=address,undefined -fno-omit-frame-pointer $(LM_CONIC_LAMBERT_SRC) $(LM_CONIC_LAMBERT_TARGET) tests/test_lm_conic_lambert.c -lm \
  -o build/test-lm-conic-lambert-sanitize
LM_CONIC_LAMBERT_TEST_PART = $(if $(filter 1,$(1)),./build/test-lm-conic-lambert && python3 tools/lm_conic_lambert_source.py && )python3 \
  tools/oracle_stream_part.py lm-conic-lambert --part $(1)/$(2) --cc "$(CC)"
LM_CONIC_LAMBERT_SANITIZE_PART = $(if $(filter 1,$(1)),$(LM_CONIC_LAMBERT_SANITIZE_BUILD) && ASAN_OPTIONS=detect_leaks=0 \
  ./build/test-lm-conic-lambert-sanitize && )python3 tools/oracle_stream_part.py lm-conic-lambert --part $(1)/$(2) --cc "$(CC)" --sanitize
$(eval $(call CI_HOST_SPLIT_RULE,test-lm-conic-lambert,4,$(LM_CONIC_LAMBERT_PART_DEPS) | build,LM_CONIC_LAMBERT_TEST_PART))
$(eval $(call CI_HOST_SPLIT_RULE,sanitize-lm-conic-lambert,4,$(LM_CONIC_LAMBERT_PART_DEPS) | build,LM_CONIC_LAMBERT_SANITIZE_PART))
test-lm-conic-lambert-1: build/test-lm-conic-lambert $(LM_CONIC_LAMBERT_HDR)
sanitize-lm-conic-lambert-1: $(LM_CONIC_LAMBERT_TARGET) tests/test_lm_conic_lambert.c $(LM_CONIC_LAMBERT_HDR)
# Nine PRGs, one per phase group, with the two images under KERNAL ROM.
c64-lm-conic-lambert: build/lm-conic-lambert-1.prg build/lm-conic-lambert-2.prg build/lm-conic-lambert-3.prg build/lm-conic-lambert-4.prg build/lm-conic-lambert-5.prg build/lm-conic-lambert-6.prg build/lm-conic-lambert-7.prg build/lm-conic-lambert-8.prg build/lm-conic-lambert-9.prg
build/lm-conic-lambert-%.prg: $(LM_CONIC_LAMBERT_SRC) $(LM_CONIC_LAMBERT_C64) platform/c64/lm_conic_lambert_diagnostic.c $(LM_CONIC_LAMBERT_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DAPOLLO_FIXED_LM_ONLY -DLM_CONIC_LAMBERT_GROUP=$* -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-conic-lambert-$*.map $(LM_CONIC_LAMBERT_SRC) $(LM_CONIC_LAMBERT_C64) platform/c64/lm_conic_lambert_diagnostic.c -o $@
	python3 -c "import re,sys;m=open('build/lm-conic-lambert-$*.map').read();e=max(int(x,16) for x in re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+[0-9a-f]+\s+\d+\s+__heap_start',m,re.M));sys.exit(0 if e<=0xc000 else 'image and BSS reach the mailbox page: '+hex(e))"
vice-lm-conic-lambert: c64-lm-conic-lambert
	python3 tools/vice_run.py build/lm-conic-lambert-1.prg
	python3 tools/vice_run.py build/lm-conic-lambert-2.prg
	python3 tools/vice_run.py build/lm-conic-lambert-3.prg
	python3 tools/vice_run.py build/lm-conic-lambert-4.prg
	python3 tools/vice_run.py build/lm-conic-lambert-5.prg
	python3 tools/vice_run.py build/lm-conic-lambert-6.prg
	python3 tools/vice_run.py build/lm-conic-lambert-7.prg
	python3 tools/vice_run.py build/lm-conic-lambert-8.prg
	python3 tools/vice_run.py build/lm-conic-lambert-9.prg
