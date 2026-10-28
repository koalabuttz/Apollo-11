# SPDX-License-Identifier: MIT
# tools/ci_run.py: a stuck target is stopped and keeps its output.
.PHONY: test-ci-run
test: test-ci-run
test-ci-run:
	python3 tests/test_ci_run.py
