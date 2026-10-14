# SPDX-License-Identifier: MIT
# Milestone lm-initvel (lane E): Translate Luminary INITVEL (P34-35 required velocity, with LAMBERT)
LM_INITVEL_SRC = src/lm_initvel.c src/lm_conic_lambert.c src/lm_conic_timethet.c src/lm_conic_kepler.c src/sband_trig.c src/guidance_trig.c src/guidance_product.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/rtb_agreement.c src/fixed_data.c src/agc_word.c src/agc_multiword.c
LM_INITVEL_TARGET = tests/lm_initvel_target.c tests/lm_initvel_phase_target.c tests/lm_initvel_cases.c tests/lm_initvel_debris.c tests/frozen_words.c
LM_INITVEL_C64 = tests/lm_initvel_phase_target.c tests/lm_initvel_debris.c tests/frozen_words.c
LM_INITVEL_HDR = include/lm_initvel.h include/lm_conic_lambert.h include/sband_trig.h include/lm_conic_timethet.h include/lm_conic_kepler.h include/lm_interp_orders.h include/lm_interp_steps.h include/agc_image_ops.h tests/lm_initvel_target.h tests/lm_initvel_vectors.h tests/lm_initvel_phase_target.h tests/lm_initvel_g1_vectors.h tests/lm_initvel_g2_vectors.h tests/lm_initvel_g3_vectors.h tests/lm_initvel_g4_vectors.h tests/lm_initvel_cases.h
.PHONY: test-lm-initvel sanitize-lm-initvel c64-lm-initvel vice-lm-initvel
all: build/test-lm-initvel
# Oracle stream: unchanged Luminary099 in the pinned yaAGC, checked row by row
# against the native producer and against the pinned stream identity.
build/lm-initvel-reference.snapshots.bin.xz: tools/lm_initvel_differential.py tools/lm_initvel_reference.c $(LM_INITVEL_SRC) tests/lm_initvel_cases.c tests/lm_initvel_debris.c | build
	python3 tools/lm_initvel_differential.py
build/test-lm-initvel: $(LM_INITVEL_SRC) $(LM_INITVEL_TARGET) tests/test_lm_initvel.c $(LM_INITVEL_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_INITVEL_SRC) $(LM_INITVEL_TARGET) tests/test_lm_initvel.c -lm -o $@
# The 65536 oracle entries in four parts of consecutive entries, each run in
# the pinned yaAGC and in a fresh strict (or ASAN/UBSAN) native producer,
# compared row by row and against the pinned SHA-256 of the part
# (tools/oracle_stream_part.py, evidence/lm-initvel/lm-initvel-source-stream-parts.json,
# recorded from the whole pinned stream). Part 1 also runs the frozen
# entries and phase steps (and, unsanitized, the source map).
LM_INITVEL_PART_DEPS = tools/oracle_stream_part.py tools/lm_initvel_core.py tools/lm_initvel_reference.c tools/lm_initvel_snapshot.c \
  tools/prepare_hardware_dv.py $(LM_INITVEL_SRC) tests/lm_initvel_cases.c tests/lm_initvel_debris.c tests/cm_executive_cases.c \
  evidence/lm-initvel/lm-initvel-source-stream.json evidence/lm-initvel/lm-initvel-source-stream-parts.json tools/mk/lm-initvel.mk
LM_INITVEL_SANITIZE_BUILD = $(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic \
  -fsanitize=address,undefined -fno-omit-frame-pointer $(LM_INITVEL_SRC) $(LM_INITVEL_TARGET) tests/test_lm_initvel.c -lm \
  -o build/test-lm-initvel-sanitize
LM_INITVEL_TEST_PART = $(if $(filter 1,$(1)),./build/test-lm-initvel && python3 tools/lm_initvel_source.py && )python3 \
  tools/oracle_stream_part.py lm-initvel --part $(1)/$(2) --cc "$(CC)"
LM_INITVEL_SANITIZE_PART = $(if $(filter 1,$(1)),$(LM_INITVEL_SANITIZE_BUILD) && ASAN_OPTIONS=detect_leaks=0 \
  ./build/test-lm-initvel-sanitize && )python3 tools/oracle_stream_part.py lm-initvel --part $(1)/$(2) --cc "$(CC)" --sanitize
$(eval $(call CI_HOST_SPLIT_RULE,test-lm-initvel,4,$(LM_INITVEL_PART_DEPS) | build,LM_INITVEL_TEST_PART))
$(eval $(call CI_HOST_SPLIT_RULE,sanitize-lm-initvel,4,$(LM_INITVEL_PART_DEPS) | build,LM_INITVEL_SANITIZE_PART))
test-lm-initvel-1: build/test-lm-initvel $(LM_INITVEL_HDR)
sanitize-lm-initvel-1: $(LM_INITVEL_TARGET) tests/test_lm_initvel.c $(LM_INITVEL_HDR)
# Four PRGs, one per phase group, with the two images under KERNAL ROM.
c64-lm-initvel: build/lm-initvel-1.prg build/lm-initvel-2.prg build/lm-initvel-3.prg build/lm-initvel-4.prg
build/lm-initvel-%.prg: $(LM_INITVEL_SRC) $(LM_INITVEL_C64) platform/c64/lm_initvel_diagnostic.c $(LM_INITVEL_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DAPOLLO_FIXED_LM_ONLY -DLM_INITVEL_GROUP=$* -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-initvel-$*.map $(LM_INITVEL_SRC) $(LM_INITVEL_C64) platform/c64/lm_initvel_diagnostic.c -o $@
	python3 -c "import re,sys;m=open('build/lm-initvel-$*.map').read();e=max(int(x,16) for x in re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+[0-9a-f]+\s+\d+\s+__heap_start',m,re.M));sys.exit(0 if e<=0xc000 else 'image and BSS reach the mailbox page: '+hex(e))"
vice-lm-initvel: c64-lm-initvel
	python3 tools/vice_run.py build/lm-initvel-1.prg
	python3 tools/vice_run.py build/lm-initvel-2.prg
	python3 tools/vice_run.py build/lm-initvel-3.prg
	python3 tools/vice_run.py build/lm-initvel-4.prg
