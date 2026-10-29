# SPDX-License-Identifier: MIT
# interp-loops-check: the differential's step-cap loop rule
# (tools/interp_loops_test.py, #746) on the real PERIODCH case: cm-conics'
# native runner at its step cap and 4x it, yaAGC at 4x its cycle cap. Its
# mocked part also runs in the CI ledgers job. A PR runs it through
# ci-extra-host when one of these files changes.
# The differential reads both missions' listings, which setup_cloud.sh
# assembles; a tree without them assembles them here (from the oracle
# fixtures, as setup_cloud.sh does).
.PHONY: interp-loops-check
interp-loops-check: build/fixed-data/Luminary099/assembly.lst build/fixed-data/Comanche055/assembly.lst
	python3 tools/interp_loops_test.py --real
build/fixed-data/%/assembly.lst:
	python3 -c "import sys,pathlib;sys.path.insert(0,'tools');from fixed_data_generate import assemble;assemble('$*',pathlib.Path('build/fixed-data'))"
ci-watch-interp-loops-check: tools/interp_loops_test.py tools/interp_routine_differential.py \
  tools/interp_routine_native.c tools/interp_routine_reference.c tests/interp_case.c tests/interp_case.h \
  tests/interp_loops_periodch.json
