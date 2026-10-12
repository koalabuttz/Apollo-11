# SPDX-License-Identifier: MIT
# Milestone lanec-di-cert (lane C): whole-file source map of DISPLAY_INTERFACE_ROUTINES, LM and CM.
.PHONY: test-lanec-di-cert
test: test-lanec-di-cert
test-lanec-di-cert: build/lanec-oracle tools/lanec/wholefile.py tools/lanec/symbols.py
	python3 tools/lanec/wholefile.py build/di-cert
	cmp build/di-cert/lanec-di-lm-source-map.json evidence/lanec-di-cert/lanec-di-lm-source-map.json
	cmp build/di-cert/lanec-di-cm-source-map.json evidence/lanec-di-cert/lanec-di-cm-source-map.json
