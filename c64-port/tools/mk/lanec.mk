# SPDX-License-Identifier: MIT
# Lane C shared harness: one native runner and one Virtual AGC oracle for all
# Lane C suites (tools/mk/lanec-*.mk add their sources to LANEC_SUITE_SRC).
LANEC_CORE = src/agc_word.c src/lanec.c tests/lanec_case.c tests/lanec_suites.c
LANEC_HDR = include/lanec.h include/lanec_case.h include/lanec_suites.h include/lanec_symbols.h $(LANEC_SUITE_HDR)
LANEC_YAAGC = deps/virtualagc/yaAGC
# Unused sections are dropped as in lm-exec-jobs: the Executive services take
# only the image import/export of p66_native.c (tests/lanec_p66_dispatch.c).
LANEC_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests -ffunction-sections -fdata-sections -Wl,--gc-sections
build/lanec-native: $(LANEC_CORE) $(LANEC_SUITE_SRC) $(LANEC_HDR) tools/lanec/native.c | build
	$(CC) -O2 $(LANEC_FLAGS) -Wconversion -pedantic $(LANEC_CORE) $(LANEC_SUITE_SRC) tools/lanec/native.c -o $@
build/lanec-native-sanitize: $(LANEC_CORE) $(LANEC_SUITE_SRC) $(LANEC_HDR) tools/lanec/native.c | build
	$(CC) -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all $(LANEC_FLAGS) $(LANEC_CORE) $(LANEC_SUITE_SRC) tools/lanec/native.c -o $@
# The oracle runs every DV through the pinned engine's own hardware model
# (tools/prepare_hardware_dv.py, docs/DIVIDE.md), like the other interpreter
# oracles: the fast DV path mistakes a 16-bit L of -0 for a nonzero word.
build/agc_engine_hardware_dv.o: tools/prepare_hardware_dv.py | build
	python3 tools/prepare_hardware_dv.py
build/lanec-oracle: $(LANEC_CORE) $(LANEC_SUITE_SRC) $(LANEC_HDR) tools/lanec/oracle.c build/agc_engine_hardware_dv.o | build
	$(CC) -O2 $(LANEC_FLAGS) -I$(LANEC_YAAGC) $(LANEC_CORE) $(LANEC_SUITE_SRC) tools/lanec/oracle.c \
	  build/agc_engine_hardware_dv.o $(LANEC_YAAGC)/agc_engine_init.o $(LANEC_YAAGC)/rfopen.o -o $@
