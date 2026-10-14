# SPDX-License-Identifier: MIT
# Milestone lanec-pin-heads-display (Pinball module split lane): the Pinball heads
# that stand in for the display consumers' composite modules
# (docs/LANEC-PIN-HEADS-DISPLAY.md), one suite per head family, LM and CM.
LANEC_PIN_HEAD_FAMILIES = decount dpout twoint minsec
LANEC_PIN_HEAD_SRC = tests/lanec_pin_head.c $(foreach f,$(LANEC_PIN_HEAD_FAMILIES),src/lanec_pin_head_$(f).c tests/lanec_pin_head_$(f)_cases.c)
LANEC_SUITE_SRC += $(LANEC_PIN_HEAD_SRC)
LANEC_SUITE_HDR += tests/lanec_pin_head.h
.PHONY: test-lanec-pin-head sanitize-lanec-pin-head
test: test-lanec-pin-head
sanitize: sanitize-lanec-pin-head
test-lanec-pin-head: build/lanec-native build/lanec-oracle tools/lanec/diff.py tools/lanec/symbols.py
	for f in $(LANEC_PIN_HEAD_FAMILIES); do python3 tools/lanec/diff.py pin-head-$$f --refusals || exit 1; done
sanitize-lanec-pin-head: build/lanec-native-sanitize build/lanec-oracle tools/lanec/diff.py
	for f in $(LANEC_PIN_HEAD_FAMILIES); do ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py pin-head-$$f --refusals --native build/lanec-native-sanitize || exit 1; done
