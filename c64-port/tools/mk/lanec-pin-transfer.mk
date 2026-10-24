# SPDX-License-Identifier: MIT
# Milestone lanec-pin-transfer (lane C): PINBALL_GAME_BUTTONS_AND_LIGHTS, LM and
# CM, as routines of the control-transfer ABI (each point runs the earlier
# Pinball modules that claim it, through their pw-* lanec_run adapters), with
# round trips from every genuine BANKCALL/POSTJUMP caller through
# INTER-BANK_COMMUNICATION, the display interface and the LM Executive.
LANEC_PIN_TRANSFER_SRC = src/lanec_pin_routines.c src/lanec_pin_trampoline.c src/lanec_pin_keys_num_routines.c src/lanec_pin_keys_cmd_routines.c src/lanec_pin_keys_release_routines.c src/lanec_pin_keys_clear_routines.c src/lanec_pin_keys_sign_routines.c tests/lanec_pin_transfer_cases.c
LANEC_SUITE_SRC += $(LANEC_PIN_TRANSFER_SRC)
LANEC_SUITE_HDR += include/lanec_pin_routines.h src/lanec_pin_routines.inc tests/lanec_pin_transfer_cases.inc
.PHONY: test-lanec-pin-transfer sanitize-lanec-pin-transfer
test: test-lanec-pin-transfer
sanitize: sanitize-lanec-pin-transfer
test-lanec-pin-transfer: build/lanec-native build/lanec-oracle tools/lanec/diff.py tools/lanec/routines.py
	python3 tools/lanec/routines.py pin --check
	python3 tools/lanec/diff.py pin-transfer --refusals
	python3 tools/lanec/diff.py pin-transfer-c64 --refusals
	python3 tools/lanec/diff.py pin-refuse --refusals-only
	python3 tools/lanec/diff.py pin-decdsp3 --refusals
sanitize-lanec-pin-transfer: build/lanec-native-sanitize build/lanec-oracle tools/lanec/diff.py
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py pin-transfer --refusals --native build/lanec-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py pin-transfer-c64 --refusals --native build/lanec-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py pin-refuse --refusals-only --native build/lanec-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py pin-decdsp3 --refusals --native build/lanec-native-sanitize
# The C64 round trips (pin-transfer-c64, one program per module) are built
# by tools/mk/lanec-pw.mk, which comes after every pw-* suite's makefile:
# lanec-pin-transfer-c64, lanec-pin-transfer-vice (CI: c64-/vice-lanec-pin-transfer-<n>).
# The host-only suites on the C64 (docs/MODULE-TEST.md): the frozen rows of
# pin-transfer-flight, pin-decdsp3-flight and pin-refuse-flight (the cases of
# pin-transfer, pin-decdsp3 and pin-refuse, run on the host as the harness
# runs them, tests/lanec_pin_transfer_cases.c run_flight) through the
# INTER-BANK, Pinball and keypress flight modules loaded from the REU store,
# up to 64 routines a case. c64-lanec-pin-chains builds the store.
LANEC_PIN_CHAIN_MODULES = lanec_pin_routines,lanec_pin_keys_num_routines,lanec_pin_keys_cmd_routines,lanec_pin_keys_release_routines,lanec_pin_keys_sign_routines,lanec_pin_keys_clear_routines
.PHONY: c64-lanec-pin-chains vice-lanec-pin-chains
c64-lanec-pin-chains:
	python3 tools/flight_store_build.py
vice-lanec-pin-chains: c64-lanec-pin-chains build/lanec-native tools/module_test.py tools/module_test.c
	set -e; for s in pin-transfer-flight pin-decdsp3-flight pin-refuse-flight; do \
	  build/lanec-native --states $$s > build/lanec-$$s-states.txt; \
	  build/lanec-native $$s > build/lanec-$$s-flight.txt; \
	  rows=$$(sed -n 's/^ *{\([0-9]*\)u, 0x.*/\1/p' tests/lanec_$$(echo $$s | tr - _)_c64.h | paste -sd,); \
	  for m in lm cm; do [ $$s = pin-decdsp3-flight ] && [ $$m = lm ] && continue; \
	    python3 tools/module_test.py --mission $$m --modules $(LANEC_PIN_CHAIN_MODULES),$${m}_interbank_routines --limit 64 \
	    --flight-rows --cases $$rows --states build/lanec-$$s-states.txt --oracle build/lanec-$$s-flight.txt \
	    --native build/lanec-$$s-flight.txt --name $$s; done; \
	  rm -f build/lanec-$$s-states.txt build/lanec-$$s-flight.txt; done
	@echo "PASS: VICE PAL lanec-pin-chains: the frozen C64 rows of pin-transfer, pin-decdsp3 and pin-refuse through the Pinball flight modules"
