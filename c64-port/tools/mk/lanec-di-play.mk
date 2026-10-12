# SPDX-License-Identifier: MIT
# Milestone lanec-di-play (lane C): DISPLAY_INTERFACE display job selection, LM and CM.
LANEC_DI_PLAY_SRC = src/lanec_di_play.c tests/lanec_di_play_cases.c
LANEC_SUITE_SRC += $(LANEC_DI_PLAY_SRC)
LANEC_SUITE_HDR += include/lanec_di_play.h include/lanec_di_internal.h
.PHONY: test-lanec-di-play sanitize-lanec-di-play c64-lanec-di-play vice-lanec-di-play
test: test-lanec-di-play
sanitize: sanitize-lanec-di-play
test-lanec-di-play: build/lanec-native build/lanec-oracle tools/lanec/diff.py tools/lanec/symbols.py
	python3 tools/lanec/diff.py di-play
sanitize-lanec-di-play: build/lanec-native-sanitize build/lanec-oracle tools/lanec/diff.py
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py di-play --native build/lanec-native-sanitize
c64-lanec-di-play: build/lanec-di-play.prg
build/lanec-di-play.prg: $(LANEC_CORE) $(LANEC_DI_PLAY_SRC) src/lanec_di_dsp.c src/lanec_di_mark.c $(LANEC_HDR) include/lanec_di_play.h include/lanec_di_internal.h include/lanec_di_mark.h tests/lanec_di_play_c64.h tools/lanec/c64_diag.c | build
	$(MOS_CC) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Iinclude -Itests \
	  -DLANEC_SUITE=lanec_suite_di_play -DLANEC_FROZEN='"lanec_di_play_c64.h"' -Wl,-Map,build/lanec-di-play.map \
	  $(LANEC_CORE) $(LANEC_DI_PLAY_SRC) src/lanec_di_dsp.c src/lanec_di_mark.c tools/lanec/c64_diag.c -o $@
vice-lanec-di-play: build/lanec-di-play.prg
	python3 tools/vice_run.py --cycles 900000000 build/lanec-di-play.prg
