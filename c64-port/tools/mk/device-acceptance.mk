# SPDX-License-Identifier: MIT
# tools/device_acceptance.py (each program's device_adapter_tests document
# from the host differential and C64 replay windows), on made-up results, and
# the off-nominal rows' windows against the host times of their checks.
.PHONY: test-device-acceptance
test: test-device-acceptance
test-device-acceptance:
	python3 tests/test_device_acceptance.py
	python3 tests/test_device_offnominal.py
