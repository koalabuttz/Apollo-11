# SPDX-License-Identifier: MIT
# Milestone lanec-pin-keys (lane C): Pinball CHARIN and its key labels, LM and CM.
LANEC_PIN_KEYS_SRC = src/lanec_pin_keys.c tests/lanec_pin_keys_cases.c
LANEC_SUITE_SRC += $(LANEC_PIN_KEYS_SRC)
LANEC_SUITE_HDR += include/lanec_pin_keys.h
.PHONY: test-lanec-pin-keys sanitize-lanec-pin-keys
test: test-lanec-pin-keys
sanitize: sanitize-lanec-pin-keys
test-lanec-pin-keys: build/lanec-native build/lanec-oracle tools/lanec/diff.py tools/lanec/symbols.py
	python3 tools/lanec/diff.py pin-keys --refusals
	python3 tools/lanec/diff.py pin-key-labels --refusals
sanitize-lanec-pin-keys: build/lanec-native-sanitize build/lanec-oracle tools/lanec/diff.py
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py pin-keys --refusals --native build/lanec-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py pin-key-labels --refusals --native build/lanec-native-sanitize
# No C64 program of its own: the cases come from ten key suites' generators,
# which with every key module do not fit one program. The C64 runs go
# through the module-test harness instead (docs/MODULE-TEST.md), on the
# five keypress flight modules loaded from the REU store; run make
# flight-store first.
LANEC_PIN_KEYS_MODULES = lanec_pin_keys_num_routines,lanec_pin_keys_cmd_routines,lanec_pin_keys_release_routines,lanec_pin_keys_sign_routines,lanec_pin_keys_clear_routines
.PHONY: module-test-lanec-pin-keys
module-test: module-test-lanec-pin-keys
module-test-lanec-pin-keys: build/lanec-native build/lanec-oracle tools/module_test.py tools/module_test.c
	set -e; for s in pin-keys pin-key-labels; do \
	  python3 tools/lanec/diff.py $$s --refusals; \
	  build/lanec-native --states $$s > build/lanec-$$s-states.txt; \
	  for m in lm cm; do python3 tools/module_test.py --mission $$m --modules $(LANEC_PIN_KEYS_MODULES) --limit 1 \
	    --tc-cells 72,114,115,116,117,124,125,136,144,154,156,162,1013 \
	    --states build/lanec-$$s-states.txt --oracle build/lanec-$$s-oracle.txt --native build/lanec-$$s-native.txt \
	    --name $$s; done; \
	  rm -f build/lanec-$$s-states.txt build/lanec-$$s-native.txt build/lanec-$$s-oracle.txt; done
