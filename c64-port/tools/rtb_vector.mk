# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
VECTOR_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
VECTOR_CORE = src/rtb_vector.c src/agc_word.c src/agc_multiword.c tests/rtb_vector_cases.c
all: build/rtb-vector-native build/rtb-vector-reference
build/rtb-vector-native: tools/rtb_vector_snapshot.c $(VECTOR_CORE)
	gcc -O2 $(VECTOR_FLAGS) tools/rtb_vector_snapshot.c $(VECTOR_CORE) -o $@
build/rtb-vector-reference: tools/rtb_vector_reference.c tests/rtb_vector_cases.c
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC tools/rtb_vector_reference.c tests/rtb_vector_cases.c tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	for mission in 0 1; do for entry in {0..4}; do tree=lm; if [ $$mission = 1 ]; then tree=cm; fi; build/rtb-vector-reference deps/$$tree-rtb-fixture/MAIN.agc.bin $$mission $$entry | gzip -n -9 > build/rtb-vector-$$tree-$$entry-reference.snapshots.gz; build/rtb-vector-native $$mission $$entry | gzip -n -9 > build/rtb-vector-$$tree-$$entry-native.snapshots.gz; cmp build/rtb-vector-$$tree-$$entry-reference.snapshots.gz build/rtb-vector-$$tree-$$entry-native.snapshots.gz; done; done
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(VECTOR_FLAGS) tools/rtb_vector_snapshot.c $(VECTOR_CORE) -o build/rtb-vector-sanitize
	for mission in 0 1; do for entry in {0..4}; do tree=lm; if [ $$mission = 1 ]; then tree=cm; fi; ASAN_OPTIONS=detect_leaks=0 build/rtb-vector-sanitize $$mission $$entry | gzip -n -9 > build/rtb-vector-$$tree-$$entry-sanitize.snapshots.gz; cmp build/rtb-vector-$$tree-$$entry-reference.snapshots.gz build/rtb-vector-$$tree-$$entry-sanitize.snapshots.gz; done; done
.PHONY: all test sanitize
VECTOR_MISSION ?= 0
VECTOR_ENTRY ?= 0
VECTOR_TREE = $(if $(filter 1,$(VECTOR_MISSION)),cm,lm)
check-host:
	for mission in 0 1; do for entry in {0..4}; do gcc -O2 $(VECTOR_FLAGS) -DVECTOR_MISSION=$$mission -DVECTOR_ENTRY=$$entry tests/test_rtb_vector.c tests/frozen_words.c $(VECTOR_CORE) -o build/test-rtb-vector-$$mission-$$entry; build/test-rtb-vector-$$mission-$$entry; done; done
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DRTB_VECTOR_CHECK_ONLY -DVECTOR_MISSION=$(VECTOR_MISSION) -DVECTOR_ENTRY=$(VECTOR_ENTRY) $(VECTOR_FLAGS) tests/test_rtb_vector.c tests/frozen_words.c tools/rtb_vector_diagnostic.c $(VECTOR_CORE) -Wl,-Map,build/rtb-vector-$(VECTOR_TREE)$(VECTOR_ENTRY).map -o build/rtb-vector-$(VECTOR_TREE)$(VECTOR_ENTRY).prg
vice-test: c64
	python tools/rtb_vector_vice_test.py --mission $(VECTOR_MISSION) --phase $(VECTOR_ENTRY) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR) | tee build/rtb-vector-$(VECTOR_TREE)$(VECTOR_ENTRY)-validation.log
.PHONY: check-host c64 vice-test
