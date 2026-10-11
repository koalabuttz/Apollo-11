# SPDX-License-Identifier: MIT
.PHONY: p66-application-source p66-application-sanitize p66-application-c64
p66-application-source:
	python3 tools/p66_application_source_audit.py
	python3 tools/p66_application_start_differential.py
	python3 tools/p66_application_job_differential.py
	python3 tools/p66_application_live_differential.py
p66-application-sanitize:
	python3 tools/p66_application_start_differential.py --sanitize
	python3 tools/p66_application_job_differential.py --sanitize
	python3 tools/p66_application_live_differential.py --sanitize
p66-application-c64:
	python3 tools/p66_application_live_build.py --rebuild-stages --cc "$(MOS_CC)" --c1541 "$(C1541)"
	python3 tools/p66_application_live_vice_test.py --vice "$(VICE)" --rom-dir "$(ROM_DIR)" --data-dir "$(VICE_DATA_DIR)"
