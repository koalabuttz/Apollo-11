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
# CI runs it in four parts of about 20 minutes, one per suite and mission
# (c64-/vice-lanec-pin-keys-<n>; docs/port-coverage.json vice_targets names
# the parts): 1 pin-keys LM, 2 pin-keys CM, 3 pin-key-labels LM,
# 4 pin-key-labels CM. c64-lanec-pin-keys-<n> builds its mission's store.
# lanec-pin-keys-vice (and module-test-lanec-pin-keys) run all four; the CI
# regex does not match them.
LANEC_PIN_KEYS_PARTS = pin-keys:lm pin-keys:cm pin-key-labels:lm pin-key-labels:cm
lanec_pin_keys_part = $(subst :, ,$(word $(1),$(LANEC_PIN_KEYS_PARTS)))
define LANEC_PIN_KEYS_PART
.PHONY: vice-lanec-pin-keys-$(1)
vice-lanec-pin-keys-$(1): c64-lanec-pin-keys-$(1) build/lanec-native build/lanec-oracle tools/module_test.py tools/module_test.c
	$$(call LANEC_PIN_KEYS_RUN,$(word 1,$(call lanec_pin_keys_part,$(1))),$(word 2,$(call lanec_pin_keys_part,$(1))),$(1))
endef
# $(call LANEC_PIN_KEYS_RUN,<suite>,<mission>,<part>): every case of the
# suite through the five modules on that mission's store, its scratch files
# named for the part. tools/lanec/diff.py writes the suite's host rows
# (build/lanec-<suite>-{native,oracle}.txt) and checks them against Virtual
# AGC; the check is the same for both missions, so only the LM part counts
# it (the CM part runs it --uncounted) and the parts' checks add up to the
# unsplit run's.
LANEC_PIN_KEYS_RUN = set -e; python3 tools/lanec/diff.py $(1) --refusals$(if $(filter cm,$(2)), --uncounted); \
	  build/lanec-native --states $(1) > build/lanec-pin-keys-$(3)-states.txt; \
	  python3 tools/module_test.py --mission $(2) --modules $(LANEC_PIN_KEYS_MODULES) --limit 1 \
	    --tc-cells 72,114,115,116,117,124,125,136,144,154,156,162,1013 \
	    --states build/lanec-pin-keys-$(3)-states.txt --oracle build/lanec-$(1)-oracle.txt \
	    --native build/lanec-$(1)-native.txt --name $(1); \
	  rm -f build/lanec-pin-keys-$(3)-states.txt
# The c64- parts are written out so that tools/ci_affected.py, which reads
# a .mk file's literal rules, selects them when this file changes.
.PHONY: c64-lanec-pin-keys-1 c64-lanec-pin-keys-2 c64-lanec-pin-keys-3 c64-lanec-pin-keys-4
c64-lanec-pin-keys-1:
	python3 tools/flight_store_build.py --missions lm
c64-lanec-pin-keys-2:
	python3 tools/flight_store_build.py --missions cm
c64-lanec-pin-keys-3:
	python3 tools/flight_store_build.py --missions lm
c64-lanec-pin-keys-4:
	python3 tools/flight_store_build.py --missions cm
$(foreach n,1 2 3 4,$(eval $(call LANEC_PIN_KEYS_PART,$(n))))
.PHONY: module-test-lanec-pin-keys lanec-pin-keys-vice
module-test: module-test-lanec-pin-keys
module-test-lanec-pin-keys lanec-pin-keys-vice: vice-lanec-pin-keys-1 vice-lanec-pin-keys-2 vice-lanec-pin-keys-3 \
  vice-lanec-pin-keys-4
