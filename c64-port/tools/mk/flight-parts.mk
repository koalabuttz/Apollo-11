# SPDX-License-Identifier: MIT
# Compiled-routine files linked as several C64 flight modules (spec
# 'flight_parts', docs/FLIGHT-PARTS.md): every part chained, against the
# whole build, over the file's differential cases. Host-only (yaAGC); like
# the files' differential targets it does not gate `test`.
FLIGHT_PARTS_SPECS = cm-p40-p47 lm-aostask cm-extended-verbs-file lm-interpreter cm-interpreter cm-imu-calibration
.PHONY: flight-parts-check
flight-parts-check: | build
	$(foreach s,$(FLIGHT_PARTS_SPECS),python3 tools/flight_parts_check.py $(s) &&) true
