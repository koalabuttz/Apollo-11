# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
AGREE_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
AGREE_CORE = src/rtb_agreement.c src/agc_word.c src/agc_multiword.c tests/rtb_agreement_cases.c
all: build/rtb-agreement-native build/rtb-agreement-reference
build/rtb-agreement-native: tools/rtb_agreement_snapshot.c $(AGREE_CORE)
	gcc -O2 $(AGREE_FLAGS) tools/rtb_agreement_snapshot.c $(AGREE_CORE) -o $@
build/rtb-agreement-reference: tools/rtb_agreement_reference.c tests/rtb_agreement_cases.c
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC tools/rtb_agreement_reference.c tests/rtb_agreement_cases.c tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	for mission in 0 1; do for entry in {0..3}; do if [ $$mission = 0 ] && [ $$entry -ge 2 ]; then continue; fi; tree=lm; if [ $$mission = 1 ]; then tree=cm; fi; build/rtb-agreement-reference deps/$$tree-rtb-fixture/MAIN.agc.bin $$mission $$entry | gzip -n -9 > build/rtb-agreement-$$tree-$$entry-reference.snapshots.gz; build/rtb-agreement-native $$mission $$entry | gzip -n -9 > build/rtb-agreement-$$tree-$$entry-native.snapshots.gz; cmp build/rtb-agreement-$$tree-$$entry-reference.snapshots.gz build/rtb-agreement-$$tree-$$entry-native.snapshots.gz; done; done
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(AGREE_FLAGS) tools/rtb_agreement_snapshot.c $(AGREE_CORE) -o build/rtb-agreement-sanitize
	for mission in 0 1; do for entry in {0..3}; do if [ $$mission = 0 ] && [ $$entry -ge 2 ]; then continue; fi; tree=lm; if [ $$mission = 1 ]; then tree=cm; fi; ASAN_OPTIONS=detect_leaks=0 build/rtb-agreement-sanitize $$mission $$entry | gzip -n -9 > build/rtb-agreement-$$tree-$$entry-sanitize.snapshots.gz; cmp build/rtb-agreement-$$tree-$$entry-reference.snapshots.gz build/rtb-agreement-$$tree-$$entry-sanitize.snapshots.gz; done; done
.PHONY: all test sanitize
AGREE_MISSION ?= 0
AGREE_ENTRY ?= 0
AGREE_TREE = $(if $(filter 1,$(AGREE_MISSION)),cm,lm)
check-host:
	for mission in 0 1; do for entry in {0..3}; do if [ $$mission = 0 ] && [ $$entry -ge 2 ]; then continue; fi; gcc -O2 $(AGREE_FLAGS) -DAGREE_MISSION=$$mission -DAGREE_ENTRY=$$entry tests/test_rtb_agreement.c tests/frozen_words.c $(AGREE_CORE) -o build/test-rtb-agreement-$$mission-$$entry; build/test-rtb-agreement-$$mission-$$entry; done; done
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DRTB_AGREEMENT_CHECK_ONLY -DAGREE_MISSION=$(AGREE_MISSION) -DAGREE_ENTRY=$(AGREE_ENTRY) $(AGREE_FLAGS) tests/test_rtb_agreement.c tests/frozen_words.c tools/rtb_agreement_diagnostic.c $(AGREE_CORE) -Wl,-Map,build/rtb-agreement-$(AGREE_TREE)$(AGREE_ENTRY).map -o build/rtb-agreement-$(AGREE_TREE)$(AGREE_ENTRY).prg
vice-test: c64
	python tools/rtb_agreement_vice_test.py --mission $(AGREE_MISSION) --phase $(AGREE_ENTRY) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR)
.PHONY: check-host c64 vice-test
