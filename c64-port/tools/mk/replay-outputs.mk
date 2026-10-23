# SPDX-License-Identifier: MIT
# tools/flight_scenario_vice.py --replay-outputs: the output list compare
# on made-up lists.
.PHONY: test-replay-outputs
test: test-replay-outputs
test-replay-outputs:
	python3 tests/test_replay_outputs.py
