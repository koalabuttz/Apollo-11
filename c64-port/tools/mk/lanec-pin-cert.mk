# SPDX-License-Identifier: MIT
# Milestone lanec-pin-cert (lane C): whole-file source maps of Luminary099 and
# Comanche055 PINBALL_GAME_BUTTONS_AND_LIGHTS. The maps live in the release
# store, so the check compares their digests in evidence/lanec-pin-cert/SHA256.json.
.PHONY: test-lanec-pin-cert
test: test-lanec-pin-cert
test-lanec-pin-cert: build/lanec-oracle tools/lanec/wholefile_pin.py tools/lanec/wholefile.py tools/lanec/symbols.py
	python3 tools/lanec/wholefile_pin.py build/pin-cert
	python3 -c 'import hashlib, json, sys; f = json.load(open("evidence/lanec-pin-cert/SHA256.json"))["files"]; \
	  bad = [n for n in ("lanec-pin-lm-source-map.json", "lanec-pin-cm-source-map.json") \
	         if hashlib.sha256(open("build/pin-cert/" + n, "rb").read()).hexdigest() != f[n]["sha256"]]; \
	  sys.exit("FAIL: differs from evidence/lanec-pin-cert: " + " ".join(bad)) if bad else \
	  print("PASS: both source maps match evidence/lanec-pin-cert/SHA256.json")'
