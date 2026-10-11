# SPDX-License-Identifier: MIT
.PHONY: lm-spectest-handoff-port-test lm-spectest-handoff-port-sanitize
lm-spectest-handoff-port-test:
	python3 tools/lm_spectest_handoff_port_host.py
lm-spectest-handoff-port-sanitize:
	python3 tools/lm_spectest_handoff_port_host.py --sanitize
