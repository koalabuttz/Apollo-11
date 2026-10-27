# SPDX-License-Identifier: MIT
# The interrupt hold gate (docs/acceptance/interrupt-hold.md, issue #912
# item 3): which hold the bound gates in a FLIGHT_STEAL window, on made-up
# logs. hold-flight-steal replays the overload windows at the flight's
# steal (fhd.STEAL_FLIGHT, 13%) on the host and the C64 (VICE), each gated
# on its elapsed hold against the bound.
.PHONY: test-hold-gate hold-flight-steal
test: test-hold-gate
test-hold-gate:
	python3 tests/test_hold_gate.py
hold-flight-steal:
	python3 tools/flight_scenario_vice.py --scenario cm-entry-steal13 --snap-cs 20900 --cs 600 --replay --replay-every 50 --hold
	python3 tools/flight_scenario_vice.py --scenario cm-entry-steal13 --snap-cs 22000 --cs 400 --replay --replay-every 50 --hold
	python3 tools/flight_scenario_vice.py --scenario lm-p63-steal13-nomon --snap-cs 19400 --cs 750 --replay --replay-every 50 --hold
