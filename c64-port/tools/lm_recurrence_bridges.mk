# SPDX-License-Identifier: MIT
.PHONY: lm-recurrence-bridges-source lm-recurrence-bridges-sanitize lm-recurrence-bridges-c64
lm-recurrence-bridges-source:
	python3 tools/lm_idle_start_differential.py
	python3 tools/lm_taskover_resume_differential.py
	python3 tools/lm_waitlist_dispatch_differential.py
lm-recurrence-bridges-sanitize:
	python3 tools/lm_idle_start_differential.py --sanitize
	python3 tools/lm_taskover_resume_differential.py --sanitize
	python3 tools/lm_waitlist_dispatch_differential.py --sanitize
lm-recurrence-bridges-c64:
	python3 tools/lm_recurrence_bridge_target_build.py --cc "$(MOS_CC)" --c1541 "$(C1541)"
	python3 tools/lm_recurrence_bridge_vice_test.py --vice "$(VICE)" --rom-dir "$(ROM_DIR)" --data-dir "$(VICE_DATA_DIR)"
	python3 tools/lm_waitlist_dispatch_target_build.py --cc "$(MOS_CC)" --c1541 "$(C1541)"
	python3 tools/lm_waitlist_dispatch_vice_test.py --vice "$(VICE)" --rom-dir "$(ROM_DIR)" --data-dir "$(VICE_DATA_DIR)"
