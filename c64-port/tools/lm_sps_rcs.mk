# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
SPS_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
SPS_CORE = src/lm_sps_rcs.c src/agc_word.c src/agc_multiword.c tests/lm_sps_rcs_cases.c
all: build/lm-sps-rcs-native build/lm-sps-rcs-reference
build/lm-sps-rcs-native: tools/lm_sps_rcs_snapshot.c $(SPS_CORE)
	gcc -O2 $(SPS_FLAGS) tools/lm_sps_rcs_snapshot.c $(SPS_CORE) -o $@
build/lm-sps-rcs-reference: tools/lm_sps_rcs_reference.c tests/lm_sps_rcs_cases.c
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC tools/lm_sps_rcs_reference.c tests/lm_sps_rcs_cases.c tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	build/lm-sps-rcs-reference deps/lm-sps-rcs-fixture/MAIN.agc.bin | gzip -n -9 > build/lm-sps-rcs-reference.snapshots.gz
	build/lm-sps-rcs-native | gzip -n -9 > build/lm-sps-rcs-native.snapshots.gz
	cmp build/lm-sps-rcs-reference.snapshots.gz build/lm-sps-rcs-native.snapshots.gz
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(SPS_FLAGS) tools/lm_sps_rcs_snapshot.c $(SPS_CORE) -o build/lm-sps-rcs-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/lm-sps-rcs-sanitize | gzip -n -9 > build/lm-sps-rcs-sanitize.snapshots.gz
	cmp build/lm-sps-rcs-reference.snapshots.gz build/lm-sps-rcs-sanitize.snapshots.gz
.PHONY: all test sanitize

check-host:
	gcc -O2 $(SPS_FLAGS) tests/test_lm_sps_rcs.c tests/frozen_words.c $(SPS_CORE) -o build/test-lm-sps-rcs
	build/test-lm-sps-rcs
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DLM_SPS_RCS_CHECK_ONLY $(SPS_FLAGS) tests/test_lm_sps_rcs.c tests/frozen_words.c tools/lm_sps_rcs_diagnostic.c $(SPS_CORE) -Wl,-Map,build/lm-sps-rcs0.map -o build/lm-sps-rcs0.prg
vice-test: c64
	python tools/lm_sps_rcs_vice_test.py --phase 0 --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR)
.PHONY: check-host c64 vice-test
