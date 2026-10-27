# SPDX-License-Identifier: MIT
# The interrupt hold gate (docs/acceptance/interrupt-hold.md, issue #912
# item 3): which hold a FLIGHT_STEAL window gates, and on which side
# (tests/test_hold_gate.py).
# vice-hold-flight-steal-*: the three windows at the flight's own steal
# (FLIGHT_STEAL 111 MCT a period, about 13%), replayed on the C64 with the
# hold meter; each fails when the host's or the C64's elapsed hold goes over
# the 1706 MCT bound. About 25 minutes together, so full and nightly runs
# only: they have no c64- target, so tools/ci_affected.py never picks them
# for a PR. hold-flight-steal runs all three.
.PHONY: test-hold-gate hold-flight-steal vice-hold-flight-steal-cm-20900 vice-hold-flight-steal-cm-22000 \
  vice-hold-flight-steal-lm-19400
test: test-hold-gate
test-hold-gate:
	python3 tests/test_hold_gate.py
hold-flight-steal: vice-hold-flight-steal-cm-20900 vice-hold-flight-steal-cm-22000 vice-hold-flight-steal-lm-19400
vice-hold-flight-steal-cm-20900: flight-store
	python3 tools/flight_scenario_vice.py --scenario cm-entry-steal13 --snap-cs 20900 --cs 600 --replay --replay-every 50 --hold
vice-hold-flight-steal-cm-22000: flight-store
	python3 tools/flight_scenario_vice.py --scenario cm-entry-steal13 --snap-cs 22000 --cs 400 --replay --replay-every 50 --hold
vice-hold-flight-steal-lm-19400: flight-store
	python3 tools/flight_scenario_vice.py --scenario lm-p63-steal13-nomon --snap-cs 19400 --cs 750 --replay --replay-every 50 --hold
