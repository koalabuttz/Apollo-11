P66_LAUNCH_PHASED_CORE = $(P66_LAUNCH_CORE) src/p66_launch_phased.c
.PHONY: differential-p66-launch-phased c64-p66-launch-phased
differential-p66-launch-phased:
	python3 tools/p66_launch_phased_differential.py
	python3 tools/p66_launch_phased_differential.py --sanitize
c64-p66-launch-phased:
	python3 tools/p66_application_service_build.py --cc $(MOS_CC)
	python3 tools/p66_launch_phased_target_build.py --cc $(MOS_CC) --c1541 $(C1541)
	python3 tools/p66_launch_phased_vice_test.py --vice $(VICE) --rom-dir $(ROM_DIR) --data-dir $(VICE_DATA_DIR)
