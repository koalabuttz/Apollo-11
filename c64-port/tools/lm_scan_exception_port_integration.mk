# SPDX-License-Identifier: MIT
.PHONY: lm-scan-exception-port-test lm-scan-exception-port-sanitize
lm-scan-exception-port-test:
	python3 tools/lm_scan_exception_port_host.py
lm-scan-exception-port-sanitize:
	python3 tools/lm_scan_exception_port_host.py --sanitize
