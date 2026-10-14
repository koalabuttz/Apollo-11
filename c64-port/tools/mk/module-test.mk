# SPDX-License-Identifier: MIT
# C64 module-test harness (docs/MODULE-TEST.md): a suite's cases run on the
# C64 in VICE through flight modules the flight resident loads from the REU
# store, each complete result compared with the host's. A lane adds its
# targets as prerequisites of module-test, each one running
# tools/module_test.py on its suites' rows. Needs make flight-store first;
# like flight-store-vice it does not gate `test`.
.PHONY: module-test
module-test:
