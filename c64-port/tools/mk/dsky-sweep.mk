# SPDX-License-Identifier: MIT
# The DSKY sweep on the C64 (acceptance item 6, tools/dsky_sweep_vice.py):
# the CI sample of key sequences on both flight stores in VICE, each C64 end
# state against the host run's. tools/dsky_sweep_known.json lists the cases
# that do not match yet; the check fails on any other failure and on a
# listed case that now matches. --cases all runs Lane C's full list.
.PHONY: dsky-sweep-vice
dsky-sweep-vice: flight-store build/agc_engine_hardware_dv.o
	python3 tools/dsky_sweep_vice.py --cases ci --skip-build --known tools/dsky_sweep_known.json
