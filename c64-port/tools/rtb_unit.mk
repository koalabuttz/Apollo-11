# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
RTBU_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
RTBU_INPUT = tests/rtb_unit_cases.c tests/rtb_vector_cases.c
RTBU_CORE = src/rtb_unit.c src/rtb_vector.c src/agc_word.c src/agc_multiword.c src/guidance_norm.c src/guidance_unit_phased.c src/guidance_length_phased.c $(RTBU_INPUT)
all: build/rtb-unit-native build/rtb-unit-reference
build/rtb-unit-native: tools/rtb_unit_snapshot.c $(RTBU_CORE)
	gcc -O2 $(RTBU_FLAGS) tools/rtb_unit_snapshot.c $(RTBU_CORE) -o $@
build/rtb-unit-reference: tools/rtb_unit_reference.c $(RTBU_INPUT)
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC tools/rtb_unit_reference.c $(RTBU_INPUT) tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	for mission in 0 1; do for x1 in 0 1 2; do for phase in 0 1 2; do tree=lm; if [ $$mission = 1 ]; then tree=cm; fi; build/rtb-unit-reference deps/$$tree-rtb-fixture/MAIN.agc.bin $$mission $$x1 $$phase | gzip -n -9 > build/rtb-unit-$$tree-$$x1-$$phase-reference.snapshots.gz; build/rtb-unit-native $$mission $$x1 $$phase | gzip -n -9 > build/rtb-unit-$$tree-$$x1-$$phase-native.snapshots.gz; cmp build/rtb-unit-$$tree-$$x1-$$phase-reference.snapshots.gz build/rtb-unit-$$tree-$$x1-$$phase-native.snapshots.gz; done; done; done
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(RTBU_FLAGS) tools/rtb_unit_snapshot.c $(RTBU_CORE) -o build/rtb-unit-sanitize
	for mission in 0 1; do for x1 in 0 1 2; do for phase in 0 1 2; do tree=lm; if [ $$mission = 1 ]; then tree=cm; fi; ASAN_OPTIONS=detect_leaks=0 build/rtb-unit-sanitize $$mission $$x1 $$phase | gzip -n -9 > build/rtb-unit-$$tree-$$x1-$$phase-sanitize.snapshots.gz; cmp build/rtb-unit-$$tree-$$x1-$$phase-reference.snapshots.gz build/rtb-unit-$$tree-$$x1-$$phase-sanitize.snapshots.gz; done; done; done
.PHONY: all test sanitize
RTBU_MISSION ?= 0
RTBU_CALLER ?= 0
RTBU_PHASE ?= 0
RTBU_TREE = $(if $(filter 1,$(RTBU_MISSION)),cm,lm)
check-host:
	for mission in 0 1; do for caller in 0 1 2; do for phase in 0 1 2; do gcc -O2 $(RTBU_FLAGS) -DRTBU_MISSION=$$mission -DRTBU_CALLER=$$caller -DRTBU_PHASE=$$phase tests/test_rtb_unit.c tests/frozen_words.c $(RTBU_CORE) -o build/test-rtb-unit-$$mission-$$caller-$$phase; build/test-rtb-unit-$$mission-$$caller-$$phase; done; done; done
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DRTB_UNIT_CHECK_ONLY -DRTBU_MISSION=$(RTBU_MISSION) -DRTBU_CALLER=$(RTBU_CALLER) -DRTBU_PHASE=$(RTBU_PHASE) $(RTBU_FLAGS) tests/test_rtb_unit.c tests/frozen_words.c tools/rtb_unit_diagnostic.c $(RTBU_CORE) -Wl,-Map,build/rtb-unit-$(RTBU_TREE)$(RTBU_CALLER)$(RTBU_PHASE).map -o build/rtb-unit-$(RTBU_TREE)$(RTBU_CALLER)$(RTBU_PHASE).prg
vice-test: c64
	python tools/rtb_unit_vice_test.py --mission $(RTBU_MISSION) --caller $(RTBU_CALLER) --phase $(RTBU_PHASE) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR) | tee build/rtb-unit-$(RTBU_TREE)$(RTBU_CALLER)$(RTBU_PHASE)-validation.log
.PHONY: check-host c64 vice-test
