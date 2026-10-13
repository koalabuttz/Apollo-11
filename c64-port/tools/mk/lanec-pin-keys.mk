# SPDX-License-Identifier: MIT
# Milestone lanec-pin-keys (lane C): Pinball CHARIN and its key labels, LM and CM.
LANEC_PIN_KEYS_SRC = src/lanec_pin_keys.c tests/lanec_pin_keys_cases.c
LANEC_SUITE_SRC += $(LANEC_PIN_KEYS_SRC)
LANEC_SUITE_HDR += include/lanec_pin_keys.h
.PHONY: test-lanec-pin-keys sanitize-lanec-pin-keys c64-lanec-pin-keys vice-lanec-pin-keys
test: test-lanec-pin-keys
sanitize: sanitize-lanec-pin-keys
test-lanec-pin-keys: build/lanec-native build/lanec-oracle tools/lanec/diff.py tools/lanec/symbols.py
	python3 tools/lanec/diff.py pin-keys --refusals
sanitize-lanec-pin-keys: build/lanec-native-sanitize build/lanec-oracle tools/lanec/diff.py
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py pin-keys --refusals --native build/lanec-native-sanitize
c64-lanec-pin-keys: build/lanec-pin-keys.prg
build/lanec-pin-keys.prg: $(LANEC_CORE) $(LANEC_PIN_KEYS_SRC) src/agc_multiword.c $(LANEC_HDR) include/lanec_pin_keys.h tests/lanec_pin_keys_c64.h tools/lanec/c64_diag.c | build
	$(MOS_CC) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Iinclude -Itests \
	  -DLANEC_SUITE=lanec_suite_pin_keys -DLANEC_FROZEN='"lanec_pin_keys_c64.h"' -Wl,-Map,build/lanec-pin-keys.map \
	  $(LANEC_CORE) $(LANEC_PIN_KEYS_SRC) src/agc_multiword.c tools/lanec/c64_diag.c -o $@
vice-lanec-pin-keys: build/lanec-pin-keys.prg
	python3 tools/vice_run.py --cycles 900000000 build/lanec-pin-keys.prg
