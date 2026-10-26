# SPDX-License-Identifier: MIT
# tools/acceptance_freshness.py (accepted programs against the build) and
# the bootable doc's hash table (tools/boot_acceptance.py), on made-up docs
# in a scratch git repository.
.PHONY: test-acceptance-freshness
test: test-acceptance-freshness
test-acceptance-freshness:
	python3 tests/test_acceptance_freshness.py
