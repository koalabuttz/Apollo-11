# SPDX-License-Identifier: MIT
.PHONY: lm-context-lifecycle-port-test lm-context-lifecycle-port-sanitize
lm-context-lifecycle-port-test:
	python3 tools/lm_context_lifecycle_port_host.py
lm-context-lifecycle-port-sanitize:
	python3 tools/lm_context_lifecycle_port_host.py --sanitize
