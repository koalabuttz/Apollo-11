# Explicit source-oracle and target reproduction targets. Existing source
# fixture setup and numerical reference snapshots must already be available.
.PHONY: test-p66-guidance sanitize-p66-guidance c64-p66-guidance

test-p66-guidance:
	python3 tools/p66_guidance_live_differential.py
	python3 tools/p66_guidance_pending_differential.py
	python3 tools/p66_guidance_canonical_differential.py

sanitize-p66-guidance:
	ASAN_OPTIONS=detect_leaks=0 python3 tools/p66_guidance_live_differential.py --sanitize
	ASAN_OPTIONS=detect_leaks=0 python3 tools/p66_guidance_pending_differential.py --sanitize
	ASAN_OPTIONS=detect_leaks=0 python3 tools/p66_guidance_canonical_differential.py --sanitize

c64-p66-guidance:
	python3 tools/freeze_p66_guidance.py
	python3 tools/p66_overlay_build.py --cc "$(MOS_CC)"
	python3 tools/p66_guidance_overlay_build.py --cc "$(MOS_CC)"
	python3 tools/p66_guidance_build.py --cc "$(MOS_CC)" --c1541 "$(C1541)"
