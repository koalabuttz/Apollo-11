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
# No C64 program yet: the cases come from ten key suites' generators, which
# with every key module do not fit one program. The C64 runs come with the
# flight module-test harness, which loads the keypress modules from the REU.
