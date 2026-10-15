# SPDX-License-Identifier: MIT
# Flight builds that leave interpretive code to the native interpreter
# (-DAGC_FLIGHT_INTERPRETER, docs/FLIGHT-INTERP-STOP.md): each file's flight
# chains against the original program stopped at NEWOPS, and SERVICER's two
# flight parts (docs/LM-SERVICER-FLIGHT-PARTS.md) stopped at their cut, and the
# flight builds' EXIT-return keys (tools/flight_exit_returns.py). Host-only (yaAGC);
# like the files' differential targets it does not gate `test`.
.PHONY: flight-interp-check
flight-interp-check: build/agc_engine_hardware_dv.o | build
	python3 tools/flight_interp_check.py
	python3 tools/lm_servicer_flight_parts.py
	python3 tools/flight_exit_returns.py
