# SPDX-License-Identifier: MIT
# Milestone lanec-ev-cert (lane C): whole-file source map of Luminary099 EXTENDED_VERBS.
.PHONY: test-lanec-ev-cert
test: test-lanec-ev-cert
test-lanec-ev-cert: build/lanec-oracle tools/lanec/wholefile_ev.py tools/lanec/wholefile.py tools/lanec/symbols.py
	python3 tools/lanec/wholefile_ev.py build/ev-cert
	cmp build/ev-cert/lanec-ev-lm-source-map.json evidence/lanec-ev-cert/lanec-ev-lm-source-map.json
