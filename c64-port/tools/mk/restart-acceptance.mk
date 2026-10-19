# SPDX-License-Identifier: MIT
# tools/restart_acceptance.py (each program's recovery_and_restart_tests
# document from the restart check, the C64 replays and the C64 GOJAM rows),
# on made-up results, and every restart row placed in a phase group.
.PHONY: test-restart-acceptance
test: test-restart-acceptance
test-restart-acceptance:
	python3 tests/test_restart_acceptance.py
