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
sanitize-lanec-pin-transfer: build/lanec-native-sanitize build/lanec-oracle tools/lanec/diff.py
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py pin-transfer --refusals --native build/lanec-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py pin-transfer-c64 --refusals --native build/lanec-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py pin-refuse --refusals-only --native build/lanec-native-sanitize
# The C64 round trips (pin-transfer-c64, one program per module) are built
# by tools/mk/lanec-pw.mk, which comes after every pw-* suite's makefile:
# c64-lanec-pin-transfer, vice-lanec-pin-transfer.
