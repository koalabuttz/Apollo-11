# SPDX-License-Identifier: MIT
# tools/dsky_sweep_vice.py's --keys parsing: a case's own channel part keeps
# its commas (tests/test_dsky_sweep_keys.py). No VICE.
.PHONY: test-dsky-sweep-keys
test: test-dsky-sweep-keys
test-dsky-sweep-keys:
	python3 tests/test_dsky_sweep_keys.py
