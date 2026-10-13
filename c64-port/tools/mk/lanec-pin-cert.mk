# SPDX-License-Identifier: MIT
# Milestone lanec-pin-cert (lane C): whole-file source maps of Luminary099 and
# Comanche055 PINBALL_GAME_BUTTONS_AND_LIGHTS.
.PHONY: test-lanec-pin-cert
test: test-lanec-pin-cert
test-lanec-pin-cert: build/lanec-oracle tools/lanec/wholefile_pin.py tools/lanec/wholefile.py tools/lanec/symbols.py
	python3 tools/lanec/wholefile_pin.py build/pin-cert
	cmp build/pin-cert/lanec-pin-lm-source-map.json evidence/lanec-pin-cert/lanec-pin-lm-source-map.json
	cmp build/pin-cert/lanec-pin-cm-source-map.json evidence/lanec-pin-cert/lanec-pin-cm-source-map.json
