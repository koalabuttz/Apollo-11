# SPDX-License-Identifier: MIT
# tools/device_acceptance.py (each program's device_adapter_tests document
# from the host differential and C64 replay windows), on made-up results.
.PHONY: test-device-acceptance
test: test-device-acceptance
test-device-acceptance:
	python3 tests/test_device_acceptance.py
