# SPDX-License-Identifier: MIT
# Milestone lanec-di-idle (lane C): DISPLAY_INTERFACE ENDIDLE handling and display returns, LM and CM.
LANEC_DI_IDLE_SRC = src/lanec_di_idle.c tests/lanec_di_idle_cases.c
LANEC_SUITE_SRC += $(LANEC_DI_IDLE_SRC)
LANEC_SUITE_HDR += include/lanec_di_idle.h
.PHONY: test-lanec-di-idle sanitize-lanec-di-idle c64-lanec-di-idle vice-lanec-di-idle
test: test-lanec-di-idle
sanitize: sanitize-lanec-di-idle
test-lanec-di-idle: build/lanec-native build/lanec-oracle tools/lanec/diff.py tools/lanec/symbols.py
	python3 tools/lanec/diff.py di-idle
sanitize-lanec-di-idle: build/lanec-native-sanitize build/lanec-oracle tools/lanec/diff.py
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py di-idle --native build/lanec-native-sanitize
c64-lanec-di-idle: build/lanec-di-idle.prg
build/lanec-di-idle.prg: $(LANEC_CORE) $(LANEC_DI_IDLE_SRC) $(LANEC_HDR) include/lanec_di_idle.h tests/lanec_di_idle_c64.h tools/lanec/c64_diag.c | build
	$(MOS_CC) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Iinclude -Itests \
	  -DLANEC_SUITE=lanec_suite_di_idle -DLANEC_FROZEN='"lanec_di_idle_c64.h"' -Wl,-Map,build/lanec-di-idle.map \
	  $(LANEC_CORE) $(LANEC_DI_IDLE_SRC) tools/lanec/c64_diag.c -o $@
vice-lanec-di-idle: build/lanec-di-idle.prg
	python3 tools/vice_run.py --cycles 900000000 build/lanec-di-idle.prg
