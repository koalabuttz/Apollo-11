# SPDX-License-Identifier: MIT
# Milestone lanec-di-sleep (lane C): DISPLAY_INTERFACE display sleep and wake, LM and CM.
LANEC_DI_SLEEP_SRC = src/lanec_di_sleep.c tests/lanec_di_sleep_cases.c
LANEC_SUITE_SRC += $(LANEC_DI_SLEEP_SRC)
LANEC_SUITE_HDR += include/lanec_di_sleep.h include/lanec_di_internal.h
.PHONY: test-lanec-di-sleep sanitize-lanec-di-sleep c64-lanec-di-sleep vice-lanec-di-sleep
test: test-lanec-di-sleep
sanitize: sanitize-lanec-di-sleep
test-lanec-di-sleep: build/lanec-native build/lanec-oracle tools/lanec/diff.py tools/lanec/symbols.py
	python3 tools/lanec/diff.py di-sleep
sanitize-lanec-di-sleep: build/lanec-native-sanitize build/lanec-oracle tools/lanec/diff.py
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py di-sleep --native build/lanec-native-sanitize
c64-lanec-di-sleep: build/lanec-di-sleep.prg
build/lanec-di-sleep.prg: $(LANEC_CORE) $(LANEC_DI_SLEEP_SRC) src/lanec_di_play.c src/lanec_di_dsp.c src/lanec_di_mark.c $(LANEC_HDR) include/lanec_di_sleep.h include/lanec_di_internal.h tests/lanec_di_sleep_c64.h tools/lanec/c64_diag.c | build
	$(MOS_CC) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Iinclude -Itests \
	  -DLANEC_SUITE=lanec_suite_di_sleep -DLANEC_FROZEN='"lanec_di_sleep_c64.h"' -Wl,-Map,build/lanec-di-sleep.map \
	  $(LANEC_CORE) $(LANEC_DI_SLEEP_SRC) src/lanec_di_play.c src/lanec_di_dsp.c src/lanec_di_mark.c tools/lanec/c64_diag.c -o $@
vice-lanec-di-sleep: build/lanec-di-sleep.prg
	python3 tools/vice_run.py --cycles 900000000 build/lanec-di-sleep.prg
