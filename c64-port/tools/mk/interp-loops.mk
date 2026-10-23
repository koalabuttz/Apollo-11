# SPDX-License-Identifier: MIT
# interp-loops-check: the differential's step-cap loop rule
# (tools/interp_loops_test.py, #746) on the real PERIODCH case: cm-conics'
# native runner at its step cap and 4x it, yaAGC at 4x its cycle cap. Its
# mocked part also runs in the CI ledgers job. A PR runs it through
# ci-extra-host when one of these files changes.
.PHONY: interp-loops-check
interp-loops-check:
	python3 tools/interp_loops_test.py --real
ci-watch-interp-loops-check: tools/interp_loops_test.py tools/interp_routine_differential.py \
  tools/interp_routine_native.c tools/interp_routine_reference.c tests/interp_case.c tests/interp_case.h \
  tests/interp_loops_periodch.json
