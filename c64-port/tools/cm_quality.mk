# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
CMQ_PHASE ?= 0
CMQ_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
CMQ_CORE = tests/cm_quality_cases.c src/cm_quality.c src/interpreted_square.c src/guidance_product.c src/guidance_sum.c src/guidance_shift.c src/agc_word.c src/agc_multiword.c
all:
	python3 tools/cm_quality_host.py
test:
	python3 tools/cm_quality_host.py
sanitize:
	python3 tools/cm_quality_host.py --sanitize
check-host:
	for phase in $$(seq 0 19); do gcc -O2 $(CMQ_FLAGS) -DCMQ_PHASE=$$phase tests/test_cm_quality.c tests/frozen_words.c $(CMQ_CORE) -o build/test-cm-quality-$$phase; build/test-cm-quality-$$phase; done
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DCM_QUALITY_CHECK_ONLY -DCMQ_PHASE=$(CMQ_PHASE) -DCM_QUALITY_ONLY_PHASE=$(CMQ_PHASE) $(CMQ_FLAGS) tests/test_cm_quality.c tests/frozen_words.c tools/cm_quality_diagnostic.c $(CMQ_CORE) -Wl,-Map,build/cm-quality-$(CMQ_PHASE).map -o build/cm-quality-$(CMQ_PHASE).prg
vice-test: c64
	python3 tools/cm_quality_vice_test.py --phase $(CMQ_PHASE) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR) | tee build/cm-quality-$(CMQ_PHASE)-validation.log
.PHONY: all test sanitize check-host c64 vice-test
