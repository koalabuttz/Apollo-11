# SPDX-License-Identifier: MIT
# tools/cert_precheck.py's DSKY sweep summary checks: its counts add up and
# its bytes match evidence/dsky-sweep/SHA256.json (tests/test_dsky_sweep_summary.py, #843). No VICE.
.PHONY: test-dsky-sweep-summary
test: test-dsky-sweep-summary
test-dsky-sweep-summary:
	python3 tests/test_dsky_sweep_summary.py
