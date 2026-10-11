# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
RTBP_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
RTBP_INPUT = tests/rtb_pulse_return_cases.c tests/rtb_angles_cases.c
RTBP_CORE = src/rtb_pulse_return.c src/agc_word.c $(RTBP_INPUT)
all: build/rtb-pulse-return-native build/rtb-pulse-return-reference
build/rtb-pulse-return-native: tools/rtb_pulse_return_snapshot.c $(RTBP_CORE)
	gcc -O2 $(RTBP_FLAGS) $^ -o $@
build/rtb-pulse-return-reference: tools/rtb_pulse_return_reference.c $(RTBP_INPUT)
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC $^ tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	for mission in 0 1; do tree=lm; if [ $$mission = 1 ]; then tree=cm; fi; build/rtb-pulse-return-reference deps/$$tree-rtb-fixture/MAIN.agc.bin $$mission 65536 | xz -T1 -3 > build/rtb-pulse-return-$$tree.states.xz; build/rtb-pulse-return-native $$mission 65536 | cmp - <(xz -dc build/rtb-pulse-return-$$tree.states.xz); done
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(RTBP_FLAGS) tools/rtb_pulse_return_snapshot.c $(RTBP_CORE) -o build/rtb-pulse-return-sanitize
	for mission in 0 1; do tree=lm; if [ $$mission = 1 ]; then tree=cm; fi; ASAN_OPTIONS=detect_leaks=0 build/rtb-pulse-return-sanitize $$mission 65536 | cmp - <(xz -dc build/rtb-pulse-return-$$tree.states.xz); done
.PHONY: all test sanitize
RTBP_MISSION ?= 0
RTBP_TREE = $(if $(filter 1,$(RTBP_MISSION)),cm,lm)
check-host:
	for mission in 0 1; do gcc -O2 $(RTBP_FLAGS) -DRTBP_MISSION=$$mission tests/test_rtb_pulse_return.c tests/frozen_words.c $(RTBP_CORE) -o build/test-rtb-pulse-return-$$mission; build/test-rtb-pulse-return-$$mission; done
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DRTB_PULSE_RETURN_CHECK_ONLY -DRTBP_MISSION=$(RTBP_MISSION) $(RTBP_FLAGS) tests/test_rtb_pulse_return.c tests/frozen_words.c tools/rtb_pulse_return_diagnostic.c $(RTBP_CORE) -Wl,-Map,build/rtb-pulse-return-$(RTBP_TREE).map -o build/rtb-pulse-return-$(RTBP_TREE).prg
vice-test: c64
	python tools/rtb_pulse_return_vice_test.py --mission $(RTBP_MISSION) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR) | tee build/rtb-pulse-return-$(RTBP_TREE)-validation.log
.PHONY: check-host c64 vice-test
