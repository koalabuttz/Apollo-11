# SPDX-License-Identifier: MIT
.PHONY: lm-admission-raw-port-test lm-admission-raw-port-sanitize
lm-admission-raw-port-test:
	python3 tools/lm_admission_raw_port_host.py
lm-admission-raw-port-sanitize:
	python3 tools/lm_admission_raw_port_host.py --sanitize
