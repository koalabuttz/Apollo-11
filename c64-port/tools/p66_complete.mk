# SPDX-License-Identifier: MIT
# Included by the parent after this coherent runtime packet is integrated.
.PHONY: p66-complete-source p66-complete-sanitize p66-complete-c64
p66-complete-source:
	python3 tools/p66_complete_live_differential.py --raw-control
	python3 tools/p66_descent_pending_differential.py

p66-complete-sanitize:
	python3 tools/p66_complete_live_differential.py --raw-control --sanitize
	python3 tools/p66_descent_pending_differential.py --sanitize

p66-complete-c64:
	python3 tools/p66_complete_build.py --rebuild-stages --cc "$(MOS_CC)" --c1541 "$(C1541)"
	python3 tools/p66_complete_vice_test.py --vice "$(VICE)" --rom-dir "$(ROM_DIR)" --data-dir "$(VICE_DATA_DIR)"
