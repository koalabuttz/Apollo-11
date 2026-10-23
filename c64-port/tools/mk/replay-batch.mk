# SPDX-License-Identifier: MIT
# tools/replay_batch.py: a list of C64 replay windows run as a batch that
# resumes (finished windows skipped, a time budget under the 2 h job limit),
# tested with a stand-in runner.
.PHONY: test-replay-batch
test: test-replay-batch
test-replay-batch:
	python3 tests/test_replay_batch.py
