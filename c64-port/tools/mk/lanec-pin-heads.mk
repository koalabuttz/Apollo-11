# SPDX-License-Identifier: MIT
# Milestone lanec-pin-heads (lane C): Pinball's own words at the points
# where an earlier module ran on through other routines in one C call
# (docs/LANEC-PIN-HEADS.md), LM and CM. One suite per head family.
LANEC_PIN_HEADS = load lamp request proceed nvsub release
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
# Pinball on the C64 as it flies (lanec-pin-flight): each suite's frozen C64
# rows (tests/lanec_<suite>_c64.h, tools/lanec/cover.py pick) run in VICE
# through the Pinball flight modules (lanec_pin_routines, chunked by
# tools/flight_store_build.py, and the five keypress modules) loaded from the
# REU store, one routine per case from its entry, and each result is compared
# with the host's run of the same routine (tools/lanec/native.c --flight,
# tools/module_test.py, docs/MODULE-TEST.md). Where the suite's own module
# accepts the case (the rows tools/lanec/diff.py checks against Virtual AGC),
# that is the suite's row; where it refuses, the try-list's next module runs.
# It runs the head families and fixed-display, which no stand-alone C64
# program holds (lanec-pin-transfer's LANEC_PIN_C64_TOO_BIG).
# CI runs it in two parts, one per mission (c64-/vice-lanec-pin-flight-<n>:
# 1 LM, 2 CM; docs/port-coverage.json vice_targets names the parts), so each
# builds only its own mission's store (c64-lanec-pin-flight-<n>; the store
# needs build/fixed-data). lanec-pin-flight-vice runs both; the CI regex
# does not match it.
LANEC_PIN_FLIGHT_SUITES = $(foreach h,$(LANEC_PIN_HEADS) $(LANEC_PIN_HEAD_FAMILIES),pin-head-$(h)) pw-fixed-display
LANEC_PIN_FLIGHT_MODULES = lanec_pin_routines,lanec_pin_keys_num_routines,lanec_pin_keys_cmd_routines,lanec_pin_keys_release_routines,lanec_pin_keys_sign_routines,lanec_pin_keys_clear_routines
LANEC_PIN_FLIGHT_PARTS = lm cm
define LANEC_PIN_FLIGHT_PART
.PHONY: vice-lanec-pin-flight-$(1)
vice-lanec-pin-flight-$(1): c64-lanec-pin-flight-$(1) build/lanec-native build/lanec-oracle tools/module_test.py tools/module_test.c
	set -e; for s in $$(LANEC_PIN_FLIGHT_SUITES); do \
	  build/lanec-native --states $$$$s > build/lanec-$$$$s-$(2)-states.txt; \
	  build/lanec-native --flight $$$$s > build/lanec-$$$$s-$(2)-flight.txt; \
	  rows=$$$$(sed -n 's/^ *{\([0-9]*\)u, 0x.*/\1/p' tests/lanec_$$$$(echo $$$$s | tr - _)_c64.h | paste -sd,); \
	  python3 tools/module_test.py --mission $(2) --modules $$(LANEC_PIN_FLIGHT_MODULES) --limit 1 \
	    --flight-rows --cases $$$$rows --states build/lanec-$$$$s-$(2)-states.txt --oracle build/lanec-$$$$s-$(2)-flight.txt \
	    --native build/lanec-$$$$s-$(2)-flight.txt --name $$$$s; \
	  rm -f build/lanec-$$$$s-$(2)-states.txt build/lanec-$$$$s-$(2)-flight.txt; done
	@echo "PASS: VICE PAL lanec-pin-flight-$(1): the frozen C64 $(2) rows of $$(words $$(LANEC_PIN_FLIGHT_SUITES)) suites through the Pinball flight modules"
endef
# The c64- parts are written out so that tools/ci_affected.py, which reads
# a .mk file's literal rules, selects them when this file changes.
.PHONY: c64-lanec-pin-flight-1 c64-lanec-pin-flight-2
c64-lanec-pin-flight-1:
	python3 tools/flight_store_build.py --missions lm
c64-lanec-pin-flight-2:
	python3 tools/flight_store_build.py --missions cm
$(eval $(call LANEC_PIN_FLIGHT_PART,1,lm))
$(eval $(call LANEC_PIN_FLIGHT_PART,2,cm))
.PHONY: lanec-pin-flight-vice
lanec-pin-flight-vice: vice-lanec-pin-flight-1 vice-lanec-pin-flight-2
