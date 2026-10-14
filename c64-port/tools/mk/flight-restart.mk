# SPDX-License-Identifier: MIT
# Hardware restarts mid-scenario (docs/FLIGHT-RESTART.md). flight-restart-check:
# a GOJAM (FLIGHT_GOJAM) at a clean sample point of LM P63 powered descent,
# the LM DAP hold, CM entry and P00 on both missions, the restart and the
# resumed program in lockstep with yaAGC on the host
# (tools/flight_restart_check.py, build/flight-restart/). flight-restart-vice:
# the LM ones again, then the restart on the C64 flight store in VICE from the
# state just after the GOJAM, its erasable image checked against yaAGC
# (platform/c64/restart_check.c, tools/flight_restart_reference.c). Like
# flight-host-differential they need the listings and the yaAGC objects, and
# flight-restart-vice the whole store; neither gates `test`.
.PHONY: flight-restart-check flight-restart-vice
flight-restart-check: build/agc_engine_hardware_dv.o | build
	python3 tools/flight_restart_check.py
flight-restart-vice: flight-store build/agc_engine_hardware_dv.o
	python3 tools/flight_restart_check.py --vice --only lm-p63-descent-restart,lm-dap-hold-restart
