# SPDX-License-Identifier: MIT
# Hardware restarts mid-scenario (docs/FLIGHT-RESTART.md). flight-restart-check:
# a GOJAM (FLIGHT_GOJAM) at a clean sample point of LM P63 powered descent,
# the LM DAP hold, CM entry and P00 on both missions, plus one restart per
# acceptance phase group (RESTART_POINTS in tools/flight_host_differential.py:
# LM P00, DOI burn, P12 ascent, P20 rendezvous; CM P00, P40 SPS burn, P64
# entry, P20, P22), the restart and the resumed program in lockstep with
# yaAGC on the host
# (tools/flight_restart_check.py, build/flight-restart/). flight-restart-vice:
# the LM P63 and DAP restarts, CM entry and CM P52 again, then the restart on the C64
# flight store in VICE from the state just after the GOJAM, its erasable
# image checked against yaAGC
# (platform/c64/restart_check.c, tools/flight_restart_reference.c). Like
# flight-host-differential they need the listings and the yaAGC objects, and
# flight-restart-vice the whole store; neither gates `test`.
# flight-gojam-vice: the same four restarts, with the GOJAM applied on the C64
# from the state just before it (the GOJAM module 0x01E5 through
# rk_run_module, checked against the post-GOJAM snapshot), then
# apollo_ov2_restart_keep_pinned and the burst against yaAGC.
.PHONY: flight-restart-check flight-restart-vice flight-gojam-vice
flight-restart-check: build/agc_engine_hardware_dv.o | build
	python3 tools/flight_restart_check.py
flight-restart-vice: flight-store build/agc_engine_hardware_dv.o
	python3 tools/flight_restart_check.py --vice --only lm-p63-descent-restart,lm-dap-hold-restart,cm-entry-restart,cm-p52-restart
flight-gojam-vice: flight-store build/agc_engine_hardware_dv.o
	python3 tools/flight_restart_check.py --vice --gojam-module --only lm-p63-descent-restart,lm-dap-hold-restart,cm-entry-restart,cm-p52-restart
