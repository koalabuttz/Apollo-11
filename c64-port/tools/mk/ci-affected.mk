# SPDX-License-Identifier: MIT
# tools/ci_affected.py: comment-only C and Python changes select no targets.
.PHONY: test-ci-affected
test: test-ci-affected
test-ci-affected:
	python3 tests/test_ci_affected.py
