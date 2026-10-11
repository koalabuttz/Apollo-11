# SPDX-License-Identifier: MIT
.PHONY: lm-core-fault-port-test lm-core-fault-port-sanitize
lm-core-fault-port-test:
	python3 tools/lm_core_fault_port_host.py
lm-core-fault-port-sanitize:
	python3 tools/lm_core_fault_port_host.py --sanitize
