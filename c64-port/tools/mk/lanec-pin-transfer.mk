# SPDX-License-Identifier: MIT
# Milestone lanec-pin-transfer (lane C): PINBALL_GAME_BUTTONS_AND_LIGHTS, LM and
# CM, as routines of the control-transfer ABI (each point runs the earlier
# Pinball modules that claim it, through their pw-* lanec_run adapters), with
# round trips from every genuine BANKCALL/POSTJUMP caller through
# INTER-BANK_COMMUNICATION, the display interface and the LM Executive.
LANEC_PIN_TRANSFER_SRC = src/lanec_pin_routines.c tests/lanec_pin_transfer_cases.c
LANEC_SUITE_SRC += $(LANEC_PIN_TRANSFER_SRC)
LANEC_SUITE_HDR += include/lanec_pin_routines.h src/lanec_pin_routines.inc tests/lanec_pin_transfer_cases.inc
.PHONY: test-lanec-pin-transfer sanitize-lanec-pin-transfer
test: test-lanec-pin-transfer
sanitize: sanitize-lanec-pin-transfer
test-lanec-pin-transfer: build/lanec-native build/lanec-oracle tools/lanec/diff.py tools/lanec/routines.py
	python3 tools/lanec/routines.py pin --check
	python3 tools/lanec/diff.py pin-transfer --refusals
	python3 tools/lanec/diff.py pin-transfer-c64 --refusals
sanitize-lanec-pin-transfer: build/lanec-native-sanitize build/lanec-oracle tools/lanec/diff.py
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py pin-transfer --refusals --native build/lanec-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py pin-transfer-c64 --refusals --native build/lanec-native-sanitize
# The C64 runs pin-transfer-c64 (every module does not fit one program): the
# INTER-BANK routines and the decimal display's points (lanec_pin_lm_c64).
LANEC_PIN_C64_SRC = src/lanec_pin_routines.c src/agc_transfer.c src/lm_interbank_routines.c src/lm_interbank.c \
  src/lanec_pw_g_runs.c src/agc_multiword.c src/dsky_pinball.c src/dsky_decimal.c \
  tests/lanec_pw.c tests/lanec_pw_decimal_cases.c tests/dsky_decimal_cases.c tests/p66_bias_cases.c \
  tests/lanec_pin_transfer_cases.c
.PHONY: c64-lanec-pin-transfer vice-lanec-pin-transfer
c64-lanec-pin-transfer: build/lanec-pin-transfer.prg
build/lanec-pin-transfer.prg: $(LANEC_CORE) $(LANEC_PIN_C64_SRC) $(LANEC_HDR) include/lanec_pin_routines.h tests/lanec_pin_transfer_c64_c64.h tools/lanec/c64_diag.c | build
	$(MOS_CC) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Iinclude -Itests -DLANEC_PIN_C64 -DLANEC_PIN_TRANSFER_C64 \
	  -DLANEC_SUITE=lanec_suite_pin_transfer_c64 -DLANEC_FROZEN='"lanec_pin_transfer_c64_c64.h"' -Wl,-Map,build/lanec-pin-transfer.map \
	  $(LANEC_CORE) $(LANEC_PIN_C64_SRC) tools/lanec/c64_diag.c -o $@
vice-lanec-pin-transfer: build/lanec-pin-transfer.prg
	python3 tools/vice_run.py --cycles 900000000 build/lanec-pin-transfer.prg
