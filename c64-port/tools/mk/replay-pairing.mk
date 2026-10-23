# SPDX-License-Identifier: MIT
# tools/flight_scenario_vice.py --replay state pairing (host and C64 states
# by centisecond, a C64 end after the host's against the host's .after
# state there), on made-up states.
.PHONY: test-replay-pairing
test: test-replay-pairing
test-replay-pairing:
	python3 tests/test_replay_pairing.py
