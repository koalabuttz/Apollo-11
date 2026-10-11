# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
RTB_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
RTB_CORE = src/rtb_angles.c src/agc_word.c src/agc_multiword.c tests/rtb_angles_cases.c
all: build/rtb-angles-native build/rtb-angles-reference
build/rtb-angles-native: tools/rtb_angles_snapshot.c $(RTB_CORE)
	gcc -O2 $(RTB_FLAGS) tools/rtb_angles_snapshot.c $(RTB_CORE) -o $@
build/rtb-angles-reference: tools/rtb_angles_reference.c tests/rtb_angles_cases.c
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC tools/rtb_angles_reference.c tests/rtb_angles_cases.c tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	for mission in 0 1; do for entry in {0..8}; do if [ $$mission = 0 ] && { [ $$entry = 6 ] || [ $$entry = 8 ]; }; then continue; fi; tree=lm; if [ $$mission = 1 ]; then tree=cm; fi; build/rtb-angles-reference deps/$$tree-rtb-fixture/MAIN.agc.bin $$mission $$entry | gzip -n -9 > build/rtb-angles-$$tree-$$entry-reference.snapshots.gz; build/rtb-angles-native $$mission $$entry | gzip -n -9 > build/rtb-angles-$$tree-$$entry-native.snapshots.gz; cmp build/rtb-angles-$$tree-$$entry-reference.snapshots.gz build/rtb-angles-$$tree-$$entry-native.snapshots.gz; done; done
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(RTB_FLAGS) tools/rtb_angles_snapshot.c $(RTB_CORE) -o build/rtb-angles-sanitize
	for mission in 0 1; do for entry in {0..8}; do if [ $$mission = 0 ] && { [ $$entry = 6 ] || [ $$entry = 8 ]; }; then continue; fi; tree=lm; if [ $$mission = 1 ]; then tree=cm; fi; ASAN_OPTIONS=detect_leaks=0 build/rtb-angles-sanitize $$mission $$entry | gzip -n -9 > build/rtb-angles-$$tree-$$entry-sanitize.snapshots.gz; cmp build/rtb-angles-$$tree-$$entry-reference.snapshots.gz build/rtb-angles-$$tree-$$entry-sanitize.snapshots.gz; done; done
.PHONY: all test sanitize
RTB_MISSION ?= 0
RTB_ENTRY ?= 0
RTB_TREE = $(if $(filter 1,$(RTB_MISSION)),cm,lm)
check-host:
	for mission in 0 1; do for entry in {0..8}; do if [ $$mission = 0 ] && { [ $$entry = 6 ] || [ $$entry = 8 ]; }; then continue; fi; gcc -O2 $(RTB_FLAGS) -DRTB_MISSION=$$mission -DRTB_ENTRY=$$entry tests/test_rtb_angles.c tests/frozen_words.c $(RTB_CORE) -o build/test-rtb-angles-$$mission-$$entry; build/test-rtb-angles-$$mission-$$entry; done; done
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DRTB_ANGLES_CHECK_ONLY -DRTB_MISSION=$(RTB_MISSION) -DRTB_ENTRY=$(RTB_ENTRY) $(RTB_FLAGS) tests/test_rtb_angles.c tests/frozen_words.c tools/rtb_angles_diagnostic.c $(RTB_CORE) -Wl,-Map,build/rtb-angles-$(RTB_TREE)$(RTB_ENTRY).map -o build/rtb-angles-$(RTB_TREE)$(RTB_ENTRY).prg
vice-test: c64
	python tools/rtb_angles_vice_test.py --mission $(RTB_MISSION) --phase $(RTB_ENTRY) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR)
.PHONY: check-host c64 vice-test
