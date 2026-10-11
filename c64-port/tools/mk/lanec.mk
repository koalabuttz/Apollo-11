# SPDX-License-Identifier: MIT
# Lane C shared harness: one native runner and one Virtual AGC oracle for all
# Lane C suites (tools/mk/lanec-*.mk add their sources to LANEC_SUITE_SRC).
LANEC_CORE = src/agc_word.c src/lanec.c tests/lanec_case.c tests/lanec_suites.c
LANEC_HDR = include/lanec.h include/lanec_case.h include/lanec_suites.h include/lanec_symbols.h $(LANEC_SUITE_HDR)
LANEC_YAAGC = deps/virtualagc/yaAGC
LANEC_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
build/lanec-native: $(LANEC_CORE) $(LANEC_SUITE_SRC) $(LANEC_HDR) tools/lanec/native.c | build
	$(CC) -O2 $(LANEC_FLAGS) -Wconversion -pedantic $(LANEC_CORE) $(LANEC_SUITE_SRC) tools/lanec/native.c -o $@
build/lanec-native-sanitize: $(LANEC_CORE) $(LANEC_SUITE_SRC) $(LANEC_HDR) tools/lanec/native.c | build
	$(CC) -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all $(LANEC_FLAGS) $(LANEC_CORE) $(LANEC_SUITE_SRC) tools/lanec/native.c -o $@
build/lanec-oracle: $(LANEC_CORE) $(LANEC_SUITE_SRC) $(LANEC_HDR) tools/lanec/oracle.c | build
	$(CC) -O2 $(LANEC_FLAGS) -I$(LANEC_YAAGC) $(LANEC_CORE) $(LANEC_SUITE_SRC) tools/lanec/oracle.c \
	  $(LANEC_YAAGC)/agc_engine.o $(LANEC_YAAGC)/agc_engine_init.o $(LANEC_YAAGC)/rfopen.o -o $@
