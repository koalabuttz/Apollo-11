# SPDX-License-Identifier: MIT
# tools/native_output_acceptance.py (each program's native_output_tests
# document from the output map, the downlink check and C64 replays), on
# made-up results, and the committed output maps.
.PHONY: test-native-output-acceptance
test: test-native-output-acceptance
test-native-output-acceptance:
	python3 tests/test_native_output_acceptance.py
