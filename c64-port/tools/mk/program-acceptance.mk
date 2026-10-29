# SPDX-License-Identifier: MIT
# tools/program_acceptance.py (each program's integrated_program_tests
# document from the coverage-map rows, host runs and C64 replays), on
# made-up results, and the committed program map's rows.
.PHONY: test-program-acceptance
test: test-program-acceptance
test-program-acceptance:
	python3 tests/test_program_acceptance.py
