# SPDX-License-Identifier: MIT
# Milestone lanec-pw (lane C): the earlier Pinball modules through the Lane C
# harness (tests/lanec_pw.h), for the Pinball full-file certificates. Each
# tools/mk/lanec-pw-*.mk adds its sources to LANEC_SUITE_SRC and its suites
# to LANEC_PW_SUITES.
LANEC_SUITE_SRC += tests/lanec_pw.c tests/lanec_pw_blanksub_cases.c src/agc_multiword.c src/dsky_pinball.c \
  src/dsky_blank.c src/dsky_blanksub.c tests/p66_bias_cases.c tests/dsky_blanksub_cases.c
LANEC_SUITE_HDR += tests/lanec_pw.h
LANEC_PW_SUITES += pw-blanksub
.PHONY: test-lanec-pw sanitize-lanec-pw
test: test-lanec-pw
sanitize: sanitize-lanec-pw
test-lanec-pw: build/lanec-native build/lanec-oracle tools/lanec/diff.py
	for s in $(LANEC_PW_SUITES); do python3 tools/lanec/diff.py $$s --refusals || exit 1; done
sanitize-lanec-pw: build/lanec-native-sanitize build/lanec-oracle tools/lanec/diff.py
	for s in $(LANEC_PW_SUITES); do ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py $$s --refusals --native build/lanec-native-sanitize || exit 1; done
