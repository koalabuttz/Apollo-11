# SPDX-License-Identifier: MIT
# Milestone lanec-pin-heads (lane C): Pinball's own words at the points
# where an earlier module ran on through other routines in one C call
# (docs/LANEC-PIN-HEADS.md), LM and CM. One suite per head family.
LANEC_PIN_HEADS = load lamp request
LANEC_PIN_HEADS_SRC = $(foreach h,$(LANEC_PIN_HEADS),src/lanec_pin_head_$(h).c tests/lanec_pin_head_$(h)_cases.c) \
  tests/lanec_pin_head_cases.c
LANEC_SUITE_SRC += $(LANEC_PIN_HEADS_SRC)
LANEC_SUITE_HDR += include/lanec_pin_heads.h tests/lanec_pin_head_cases.h
.PHONY: test-lanec-pin-heads sanitize-lanec-pin-heads
test: test-lanec-pin-heads
sanitize: sanitize-lanec-pin-heads
test-lanec-pin-heads: build/lanec-native build/lanec-oracle tools/lanec/diff.py tools/lanec/symbols.py
	for h in $(LANEC_PIN_HEADS); do python3 tools/lanec/diff.py pin-head-$$h --refusals || exit 1; done
sanitize-lanec-pin-heads: build/lanec-native-sanitize build/lanec-oracle tools/lanec/diff.py
	for h in $(LANEC_PIN_HEADS); do ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py pin-head-$$h --refusals \
	  --native build/lanec-native-sanitize || exit 1; done
