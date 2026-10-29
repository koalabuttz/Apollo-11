# SPDX-License-Identifier: MIT
DSKY_MACHINE_CADR_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_MACHINE_CADR_CORE = src/agc_word.c src/dsky_pinball.c src/dsky_octal_display.c src/dsky_machine_cadr.c tests/p66_bias_cases.c tests/dsky_machine_cadr_cases.c
DSKY_MACHINE_CADR_TEST = tests/dsky_machine_cadr_check.c tests/test_dsky_machine_cadr.c
all: build/test-dsky-machine-cadr
build/test-dsky-machine-cadr: $(DSKY_MACHINE_CADR_CORE) $(DSKY_MACHINE_CADR_TEST) tests/dsky_machine_cadr_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_MACHINE_CADR_FLAGS) $(DSKY_MACHINE_CADR_CORE) $(DSKY_MACHINE_CADR_TEST) -o $@
test-dsky-machine-cadr: build/test-dsky-machine-cadr
	build/test-dsky-machine-cadr
sanitize-dsky-machine-cadr:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_MACHINE_CADR_FLAGS) $(DSKY_MACHINE_CADR_CORE) $(DSKY_MACHINE_CADR_TEST) -o build/test-dsky-machine-cadr-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-machine-cadr-sanitize
c64-dsky-machine-cadr:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_MACHINE_CADR_FLAGS) $(DSKY_MACHINE_CADR_CORE) tests/dsky_machine_cadr_check.c tools/dsky_machine_cadr_diagnostic.c -Wl,-Map,build/dsky_machine_cadr.map -o build/dsky_machine_cadr.prg
differential-dsky-machine-cadr:
	python3 tools/dsky_machine_cadr_differential.py
	python3 tools/freeze_dsky_machine_cadr.py
test: test-dsky-machine-cadr
sanitize: sanitize-dsky-machine-cadr
.PHONY: all test sanitize test-dsky-machine-cadr sanitize-dsky-machine-cadr c64-dsky-machine-cadr differential-dsky-machine-cadr
